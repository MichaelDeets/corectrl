// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "config.h"
#include "singleinstance.h"
#include <QCommandLineParser>
#include <QObject>
#include <QRect>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariant>
#include <memory>
#include <string_view>

class IHelperControl;
class ISession;
class ISysModelSyncer;
class IUIFactory;
class QApplication;
class QQmlApplicationEngine;
class QQuickWindow;
class QTranslator;
class Settings;
class SysTray;

class AppInfo final : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString name READ getName CONSTANT)
  Q_PROPERTY(QString version READ getVersion CONSTANT)

 public:
  AppInfo(std::string_view appName, std::string_view appVersion) noexcept
  : QObject()
  , name_(appName.data())
  , version_(appVersion.data())
  {
  }

  QString const &getName() const
  {
    return name_;
  }

  QString const &getVersion() const
  {
    return version_;
  }

 private:
  QString const name_;
  QString const version_;
};

class App final : public QObject
{
  Q_OBJECT

 public:
  static constexpr std::string_view Name{PROJECT_NAME};
  static constexpr std::string_view VersionStr{PROJECT_VERSION};
  static constexpr std::string_view Fqdn{PROJECT_FQDN};
  static constexpr QRect DefaultWindowGeometry{0, 0, 970, 600};

  App() noexcept;
  ~App();

  int exec(int argc, char **argv);

 private slots:
  void exit();
  void showMainWindow(bool show);
  void onNewInstance(QStringList args);
  void onSysTrayActivated();
  void onSettingChanged(QString const &key, QVariant const &value);
  void saveMainWindowGeometry();

 private:
  std::unique_ptr<QApplication> createApplication(int &argc, char **argv);
  bool buildComponents(QQmlApplicationEngine &qmlEngine, int helperTimeout);
  void buildUI(std::unique_ptr<IUIFactory> &&uiFactory,
               QQmlApplicationEngine &qmlEngine);
  void loadTranslation(QApplication &app, QTranslator &translator);
  int helperTimeout(int defaultTimeout) const;
  void setupCmdParser(QApplication &app, int defaultHelperTimeout);
  void setupSysTrayWindowState();
  void setupMainWindowGeometry();
  void restoreMainWindowGeometry();
  bool handleManualProfileCmd();
  bool handleWindowVisibilityCmds();

  AppInfo appInfo_;
  SingleInstance singleInstance_;
  QCommandLineParser cmdParser_;
  QTimer geometrySaveTimer_;

  std::unique_ptr<IHelperControl> helperControl_;
  std::shared_ptr<ISysModelSyncer> sysSyncer_;
  std::unique_ptr<ISession> session_;
  std::unique_ptr<Settings> settings_;

  bool noop_{false};
  QQuickWindow *mainWindow_{nullptr};
  SysTray *sysTray_{nullptr};
};
