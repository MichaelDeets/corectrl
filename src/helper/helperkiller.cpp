// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "helperkiller.h"

#include "helperids.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <exception>
#include <signal.h>
#include <string>

HelperKiller::HelperKiller(QObject *parent) noexcept
: QDBusAbstractAdaptor(parent)
{
}

HelperKiller::~HelperKiller() = default;

bool HelperKiller::start(QDBusMessage const &message) const
{
  QProcess cmd;
  cmd.start(QStringLiteral("pidof"), QStringList(HELPER_EXE));

  bool result = cmd.waitForFinished();
  if (result) {
    try {
      auto output = cmd.readAllStandardOutput().toStdString();
      int pid = std::stoi(output);
      result = kill(pid, SIGKILL) == 0;
    }
    catch (std::exception const &) {
      result = false;
    }
  }

  QTimer::singleShot(0, QCoreApplication::instance(), &QCoreApplication::quit);

  return result;
}

bool initDBusForHelperKillerService(QObject *obj)
{
  QDBusConnection bus = QDBusConnection::systemBus();
  return bus.isConnected() &&
         bus.registerObject(QStringLiteral(DBUS_HELPER_KILLER_PATH), obj) &&
         bus.registerService(QStringLiteral(DBUS_HELPER_KILLER_SERVICE));
}

bool endDBusForHelperKillerService()
{
  QDBusConnection bus = QDBusConnection::systemBus();
  bus.unregisterObject(QStringLiteral(DBUS_HELPER_KILLER_PATH));
  return bus.unregisterService(QStringLiteral(DBUS_HELPER_KILLER_SERVICE));
}

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  new HelperKiller(&app);

  if (!initDBusForHelperKillerService(&app))
    exit(1);

  app.exec();

  if (!endDBusForHelperKillerService())
    exit(1);

  return 0;
}
