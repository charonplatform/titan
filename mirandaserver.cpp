// Copyright 2016 The Arken Platform Authors.
// All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <mirandaserver.h>
#include <mirandatask.h>
#include <arken/base>
#include <QDebug>
#include <QFile>
#include <QJsonObject>

using arken::service;
using arken::Log;

MirandaServer::MirandaServer(QCoreApplication *app)
{

  QFile config("config/miranda.json");
  if( config.exists() ) {
    config.open(QIODevice::ReadOnly);
    QJsonParseError * error = new QJsonParseError();
    QJsonDocument json = QJsonDocument::fromJson(config.readAll(), error);
    if( error->error != 0 ) {
      qDebug() << error->errorString();
      throw;
    }
    QJsonObject object = json.object();
    m_port           = object.value("port").toInt();
    m_address        = object.value("address").toString();
    m_maxThreadCount = object.value("threads").toInt();
    m_pid            = object.value("pid").toString();
    m_service        = object.value("service").toBool();
  } else {
    qDebug() << "config/miranda.json file not exists";
    m_port           = 2345;
    m_address        = "localhost";
    m_maxThreadCount = 15;
    m_pid            = "miranda.pid";
    m_service        = true;
  }

  QFileInfo dispatch = QFileInfo("dispatch.lua");

  if( ! dispatch.exists() ) {
    if( app->arguments().size() == 1 ) {
      qDebug() << "where the dispatch.lua file ???";
      throw;
    }
    QString file_name = app->arguments().at(1);
    dispatch = QFileInfo(file_name);
    if( dispatch.exists() ) {
      qDebug() << "work dir is: " << dispatch.absoluteDir().absolutePath();
      QDir::setCurrent( dispatch.absoluteDir().absolutePath() );
    } else {
      qDebug() << "file dispatch.lua not found exit... ";
      throw;
    }
  }

  // SERVICES
  if( m_service && os::exists("app/services")) {
    arken::service::load("app/services");
  }

  if( os::exists("logs") ) {
    Log log = Log("logs/miranda.log");
    log.info("iniciando miranda");
    if( m_service ) {
      if( os::exists("app/services") ) {
        log.info("services started");
      } else {
        log.info("services dir not exists");
      }
    } else {
      log.info("services not enable");
    }
    log.dump();
  }

}

void MirandaServer::start()
{
  m_pool = new QThreadPool(this);
  m_pool->setMaxThreadCount(m_maxThreadCount);

  if(this->listen(QHostAddress(m_address), m_port)) {
    qDebug() << QString("start miranda %1:%2 (%3) threads...").arg(m_address).arg(m_port).arg(m_maxThreadCount).toLocal8Bit().data();
  } else {
    qDebug() << "fail start miranda ...";
    throw;
  }
  QFile log(m_pid);
  if ( log.open(QIODevice::WriteOnly) ) {
    log.write(QByteArray::number((qint64) os::pid()));
    log.close();
  } else {
    qDebug() << m_pid << " not open";
    throw;
  }
}

void MirandaServer::incomingConnection(qintptr descriptor)
{
  m_pool->start(new MirandaTask(descriptor));
}
