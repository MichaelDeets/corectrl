// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "app.h"

#include "common/stringutils.h"
#include "core/isession.h"
#include "core/isysmodelsyncer.h"
#include "core/iuifactory.h"
#include "corefactory.h"
#include "helper/ihelpercontrol.h"
#include "settings.h"
#include "systray.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTranslator>
#include <Qt>
#include <QtGlobal>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <units.h>
#include <utility>

#if defined(_DEBUG)
#include <QQmlDebuggingEnabler>
#endif

App::App() noexcept
: QObject()
, appInfo_(App::Name, App::VersionStr)
, singleInstance_(App::Name)
{
}

App::~App() = default;

int App::exec(int argc, char **argv)
{
  auto defaultHelperTimeout = std::max(
      180000, // default helper timeout in milliseconds
      IHelperControl::MinExitTimeout::value().to<int>());

  auto app = createApplication(argc, argv);
  setupCmdParser(*app, defaultHelperTimeout);

  noop_ = cmdParser_.isSet("help") || cmdParser_.isSet("version");
  if (noop_)
    return 0;

  // Exit if there is another instance running.
  if (!singleInstance_.mainInstance(app->arguments()))
    return 0;

  QTranslator translator;
  loadTranslation(*app, translator);

  QQmlApplicationEngine qmlEngine;
  bool logCommands = cmdParser_.isSet("enable-log-commands");
  bool logProfileStack = cmdParser_.isSet("enable-log-profile-stack");
  if (!buildComponents(qmlEngine, helperTimeout(defaultHelperTimeout),
                       logCommands, logProfileStack))
    return -1;

  // Load and apply stored settings.
  settings_->signalSettings();

  setupSysTrayWindowState();
  handleManualProfileCmd();

  return app->exec();
}

void App::exit()
{
  if (!noop_) {
    sysSyncer_->stop();
    helperControl_->stop();

    // Shutdown spdlog before QApplication quits to always flush the logs.
    // See: https://github.com/gabime/spdlog/issues/2502
    spdlog::shutdown();
  }
}

void App::showMainWindow(bool show)
{
  if (show) {
    mainWindow_->show();
    mainWindow_->raise();
    mainWindow_->requestActivate();
  }
  else {
    if (sysTray_->isVisible())
      mainWindow_->hide();
    else
      mainWindow_->showMinimized();
  }
}

void App::onNewInstance(QStringList args)
{
  cmdParser_.parse(args);

  bool runtimeCmds{false};
  runtimeCmds |= handleLoggingCmds();
  runtimeCmds |= handleManualProfileCmd();
  runtimeCmds |= handleWindowVisibilityCmds();

  // No runtime commands were used as arguments.
  // Show the main window unconditionally.
  if (!runtimeCmds)
    showMainWindow(true);
}

void App::onSysTrayActivated()
{
  showMainWindow(!mainWindow_->isVisible());
}

void App::onSettingChanged(QString const &key, QVariant const &value)
{
  sysTray_->settingChanged(key, value);
  sysSyncer_->settingChanged(key, value);
}

std::unique_ptr<QApplication> App::createApplication(int &argc, char **argv)
{
  // Ignore QT_STYLE_OVERRIDE. It breaks the QML theme.
  if (qEnvironmentVariableIsSet("QT_STYLE_OVERRIDE")) {
    SPDLOG_INFO("Ignoring QT_STYLE_OVERRIDE environment variable.");
    qunsetenv("QT_STYLE_OVERRIDE");
  }

  QCoreApplication::setApplicationName(QString(App::Name.data()).toLower());
  QCoreApplication::setApplicationVersion(App::VersionStr.data());
  QGuiApplication::setDesktopFileName(QString(App::Fqdn.data()));

  auto app = std::make_unique<QApplication>(argc, argv);
  app->setWindowIcon(QIcon::fromTheme(QString(App::Name.data()).toLower()));

  // Ensure that the application do not implicitly call to quit after closing
  // the last window, which is not the desired behaviour when minimize to
  // system tray is being used.
  app->setQuitOnLastWindowClosed(false);

  return app;
}

bool App::buildComponents(QQmlApplicationEngine &qmlEngine, int helperTimeout,
                          bool logCommands, bool logProfileStack)
{
  try {
    auto core = CoreFactory().build(std::string(App::Name), logCommands,
                                    logProfileStack);
    if (!core)
      return false;

    std::swap(helperControl_, core->helperControl);
    std::swap(sysSyncer_, core->sysSyncer);
    std::swap(session_, core->session);

    settings_ = std::make_unique<Settings>(QString(App::Name.data()).toLower());

    helperControl_->init(units::time::millisecond_t(helperTimeout));
    sysSyncer_->init();
    session_->init(sysSyncer_->sysModel());

    buildUI(std::move(core->uiFactory), qmlEngine);
  }
  catch (std::exception const &e) {
    SPDLOG_WARN(e.what());
    SPDLOG_WARN("Initialization failed");
    SPDLOG_WARN("Exiting...");
    return false;
  }

  return true;
}

void App::buildUI(std::unique_ptr<IUIFactory> &&uiFactory,
                  QQmlApplicationEngine &qmlEngine)
{
  qmlEngine.rootContext()->setContextProperty("appInfo", &appInfo_);
  qmlEngine.rootContext()->setContextProperty("settings", &*settings_);

  uiFactory->build(qmlEngine, sysSyncer_->sysModel(), *session_);

  mainWindow_ = qobject_cast<QQuickWindow *>(qmlEngine.rootObjects().value(0));
  setupMainWindowGeometry();

  connect(&qmlEngine, &QQmlApplicationEngine::quit, QApplication::instance(),
          &QApplication::quit);
  connect(QApplication::instance(), &QApplication::aboutToQuit, this, &App::exit);
  connect(&*settings_, &Settings::settingChanged, this, &App::onSettingChanged);
  connect(&singleInstance_, &SingleInstance::newInstance, this,
          &App::onNewInstance);

  sysTray_ = new SysTray(&*session_, mainWindow_);
  connect(sysTray_, &SysTray::quit, this, &QApplication::quit);
  connect(sysTray_, &SysTray::activated, this, &App::onSysTrayActivated);
  connect(sysTray_, &SysTray::showMainWindowToggled, this, &App::showMainWindow);
  connect(mainWindow_, &QQuickWindow::visibleChanged, &*sysTray_,
          &SysTray::onMainWindowVisibleChanged);
  qmlEngine.rootContext()->setContextProperty("systemTray", sysTray_);
}

void App::loadTranslation(QApplication &app, QTranslator &translator)
{
  QString lang = cmdParser_.isSet("lang") ? cmdParser_.value("lang")
                                          : QLocale().system().name();
  if (!translator.load(QStringLiteral(":/translations/lang_") + lang)) {
    SPDLOG_INFO("No translation found for locale {}", lang.toStdString());
    SPDLOG_INFO("Using en_EN translation.");
    if (!translator.load(QStringLiteral(":/translations/lang_en_EN")))
      SPDLOG_ERROR("Cannot load en_EN translation.");
  }
  app.installTranslator(&translator);
}

int App::helperTimeout(int defaultTimeout) const
{
  int value = defaultTimeout;
  if (cmdParser_.isSet("helper-timeout") &&
      Utils::String::toNumber<int>(
          value, cmdParser_.value("helper-timeout").toStdString())) {
    value = std::max(IHelperControl::MinExitTimeout::value().to<int>(), value);
  }
  return value;
}

void App::setupCmdParser(QApplication &app, int defaultHelperTimeout)
{
  cmdParser_.addHelpOption();
  cmdParser_.addVersionOption();
  cmdParser_.addOptions({
      {{"l", "lang"},
       "Forces a specific <language>, given in locale format. Example: "
       "en_EN.",
       "language"},
      {{"m", "toggle-manual-profile"},
       "Activate the manual profile whose name is <\"profile name\">.\nWhen an "
       "instance of the application is already running, it will toggle "
       "the manual profile whose name is <\"profile name\">.",
       "\"profile name\""},
      {"activate-manual-profile",
       "Activate the manual profile whose name is <\"profile name\">.\nWhen an "
       "instance of the application is already running, it will activate "
       "the manual profile whose name is <\"profile name\">.",
       "\"profile name\""},
      {"deactivate-manual-profile",
       "Deactivate the manual profile whose name is <\"profile name\">.\nWhen "
       "an instance of the application is already running, it will deactivate "
       "the manual profile whose name is <\"profile name\">.",
       "\"profile name\""},
      {"minimize-systray",
       "Minimizes the main window either to the system tray (when "
       "available) or to the taskbar.\nWhen an instance of the application is "
       "already running, the action will be applied to its main window."},
      {{"t", "helper-timeout"},
       "Sets helper auto exit timeout. "
       "The helper process kills himself when no signals are received from "
       "the application before the timeout expires.\nValues lesser than " +
           QString::number(IHelperControl::MinExitTimeout::value().to<int>()) +
           +" milliseconds will be ignored.\nDefault value: " +
           QString::number(defaultHelperTimeout) + " milliseconds.",
       "milliseconds"},
      {"toggle-window-visibility",
       "When an instance of the application is already running, it will toggle "
       "the main window visibility showing or minimizing it, either to the "
       "taskbar or to system tray."},
      {"enable-log-commands",
       "Enables logging of control commands. It takes precedence over "
       "disable-log-commands.\nIt can be used to activate commands logging on "
       "a running instance of the application or while starting it."},
      {"disable-log-commands",
       "Disables logging of control commands.\nIt can be used to deactivate "
       "commands logging on a running instance of the application or while "
       "starting it (no-op)."},
      {"enable-log-profile-stack",
       "Enables logging of the profile stack. It takes precedence over "
       "disable-log-profile-stack.\nIt can be used to activate the profile "
       "stack logging on a running instance of the application or while "
       "starting it."},
      {"disable-log-profile-stack",
       "Disables logging of the profile stack.\nIt can be used to deactivate "
       "the profile stack logging on a running instance of the application or "
       "while starting it (no-op)."},
  });
  cmdParser_.process(app);
}

void App::setupSysTrayWindowState()
{
  bool minimizeArgIsSet = cmdParser_.isSet("minimize-systray");
  bool enableSysTray = settings_->getValue("sysTray", true).toBool();

  if (minimizeArgIsSet || enableSysTray)
    sysTray_->show();

  bool startOnSysTray = settings_->getValue("startOnSysTray", false).toBool();
  bool showWindow = !minimizeArgIsSet && !(sysTray_->isAvailable() &&
                                           enableSysTray && startOnSysTray);

  showMainWindow(showWindow);
}

void App::setupMainWindowGeometry()
{
  restoreMainWindowGeometry();

  // The geometry save timer is used to reduce the window geometry changed
  // events fired within a time interval into a single event that will save the
  // window geometry.
  geometrySaveTimer_.setInterval(2000);
  geometrySaveTimer_.setSingleShot(true);
  connect(&geometrySaveTimer_, &QTimer::timeout, this,
          &App::saveMainWindowGeometry);

  connect(mainWindow_, &QWindow::heightChanged, this,
          [&](int) { geometrySaveTimer_.start(); });
  connect(mainWindow_, &QWindow::widthChanged, this,
          [&](int) { geometrySaveTimer_.start(); });
  connect(mainWindow_, &QWindow::xChanged, this,
          [&](int) { geometrySaveTimer_.start(); });
  connect(mainWindow_, &QWindow::yChanged, this,
          [&](int) { geometrySaveTimer_.start(); });
}

void App::saveMainWindowGeometry()
{
  if (!settings_->getValue("saveWindowGeometry", true).toBool())
    return;

  if (mainWindow_ == nullptr)
    return;

  auto windowGeometry = mainWindow_->geometry();

  auto savedXPos =
      settings_->getValue("Window/main-x-pos", DefaultWindowGeometry.x()).toInt();
  if (savedXPos != windowGeometry.x())
    settings_->setValue("Window/main-x-pos", windowGeometry.x());

  auto savedYPos =
      settings_->getValue("Window/main-y-pos", DefaultWindowGeometry.y()).toInt();
  if (savedYPos != windowGeometry.y())
    settings_->setValue("Window/main-y-pos", windowGeometry.y());

  auto savedWidth =
      settings_->getValue("Window/main-width", DefaultWindowGeometry.width())
          .toInt();
  if (savedWidth != windowGeometry.width())
    settings_->setValue("Window/main-width", windowGeometry.width());

  auto savedHeight =
      settings_->getValue("Window/main-height", DefaultWindowGeometry.height())
          .toInt();
  if (savedHeight != windowGeometry.height())
    settings_->setValue("Window/main-height", windowGeometry.height());
}

void App::restoreMainWindowGeometry()
{
  if (mainWindow_ == nullptr)
    return;

  auto x =
      settings_->getValue("Window/main-x-pos", DefaultWindowGeometry.x()).toInt();
  auto y =
      settings_->getValue("Window/main-y-pos", DefaultWindowGeometry.y()).toInt();
  auto width =
      settings_->getValue("Window/main-width", DefaultWindowGeometry.width())
          .toInt();
  auto height =
      settings_->getValue("Window/main-height", DefaultWindowGeometry.height())
          .toInt();

  mainWindow_->setGeometry(x, y, width, height);
}

bool App::handleLoggingCmds()
{
  auto cmdHandled{false};

  if (cmdParser_.isSet("enable-log-commands")) {
    sysSyncer_->logCommands(true);
    cmdHandled = true;
  }
  else if (cmdParser_.isSet("disable-log-commands")) {
    sysSyncer_->logCommands(false);
    cmdHandled = true;
  }

  if (cmdParser_.isSet("enable-log-profile-stack")) {
    session_->logProfileStack(true);
    cmdHandled = true;
  }
  else if (cmdParser_.isSet("disable-log-profile-stack")) {
    session_->logProfileStack(false);
    cmdHandled = true;
  }

  return cmdHandled;
}

bool App::handleManualProfileCmd()
{
  auto cmdHandled{false};
  if (cmdParser_.isSet("toggle-manual-profile")) {

    auto profile = cmdParser_.value("toggle-manual-profile").toStdString();
    if (profile.empty() || profile.length() >= 512)
      SPDLOG_WARN("'{}' is not a valid manual profile name.", profile);
    else if (!session_->toggleManualProfile(profile))
      SPDLOG_WARN("Cannot toggle manual profile '{}': Missing profile or not a "
                  "manual profile.",
                  profile);

    cmdHandled = true;
  }
  else if (cmdParser_.isSet("activate-manual-profile")) {

    auto profile = cmdParser_.value("activate-manual-profile").toStdString();
    if (profile.empty() || profile.length() >= 512)
      SPDLOG_WARN("'{}' is not a valid manual profile name.", profile);
    else if (!session_->activateManualProfile(profile))
      SPDLOG_WARN(
          "Cannot activate manual profile '{}': Missing profile or not a "
          "manual profile.",
          profile);

    cmdHandled = true;
  }
  else if (cmdParser_.isSet("deactivate-manual-profile")) {

    auto profile = cmdParser_.value("deactivate-manual-profile").toStdString();
    if (profile.empty() || profile.length() >= 512)
      SPDLOG_WARN("'{}' is not a valid manual profile name.", profile);
    else if (!session_->deactivateManualProfile(profile))
      SPDLOG_WARN(
          "Cannot deactivate manual profile '{}': Missing profile or not a "
          "manual profile.",
          profile);

    cmdHandled = true;
  }

  return cmdHandled;
}

bool App::handleWindowVisibilityCmds()
{
  auto cmdHandled{false};
  auto show{false};

  // Minimize to system tray takes precedence over any other window visibility
  // command.
  if (cmdParser_.isSet("minimize-systray")) {
    cmdHandled = true;
  }
  else if (cmdParser_.isSet("toggle-window-visibility")) {

    // When the window is minimized, calling show() will raise it.
    auto minimized =
        ((mainWindow_->windowState() & Qt::WindowState::WindowMinimized) ==
         Qt::WindowState::WindowMinimized);

    show = minimized ? true : !mainWindow_->isVisible();
    cmdHandled = true;
  }

  if (cmdHandled)
    showMainWindow(show);

  return cmdHandled;
}
