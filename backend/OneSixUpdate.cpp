/* Copyright 2013 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "OneSixUpdate.h"

#include <QtNetwork>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDataStream>

#include <QDebug>

#include "BaseInstance.h"
#include "lists/MinecraftVersionList.h"
#include "VersionFactory.h"
#include "OneSixVersion.h"
#include "OneSixInstance.h"

#include "pathutils.h"


OneSixUpdate::OneSixUpdate(BaseInstance *inst, QObject *parent) :
	Task(parent)
{
	m_inst = inst;
}

void OneSixUpdate::executeTask()
{
	QString intendedVersion = m_inst->intendedVersionId();
	// Get a pointer to the version object that corresponds to the instance's version.
	targetVersion = (MinecraftVersion *)MinecraftVersionList::getMainList().findVersion(intendedVersion);
	if(targetVersion == nullptr)
	{
		// don't do anything if it was invalid
		emit gameUpdateComplete();
		return;
	}
	
	if(m_inst->shouldUpdate())
	{
		versionFileStart();
	}
	else
	{
		jarlibStart();
	}
}

void OneSixUpdate::versionFileStart()
{
	setStatus("Getting the version files from Mojang.");
	
	QString urlstr("http://s3.amazonaws.com/Minecraft.Download/versions/");
	urlstr += targetVersion->descriptor() + "/" + targetVersion->descriptor() + ".json";
	auto dljob = DownloadJob::create(QUrl(urlstr));
	specificVersionDownloadJob.reset(new JobList());
	specificVersionDownloadJob->add(dljob);
	connect(specificVersionDownloadJob.data(), SIGNAL(finished()), SLOT(versionFileFinished()));
	connect(specificVersionDownloadJob.data(), SIGNAL(failed()), SLOT(versionFileFailed()));
	connect(specificVersionDownloadJob.data(), SIGNAL(progress(qint64,qint64)), SLOT(updateDownloadProgress(qint64,qint64)));
	download_queue.enqueue(specificVersionDownloadJob);
}

void OneSixUpdate::versionFileFinished()
{
	JobPtr firstJob = specificVersionDownloadJob->getFirstJob();
	auto DlJob = firstJob.dynamicCast<DownloadJob>();
	
	QString version_id = targetVersion->descriptor();
	QString inst_dir = m_inst->rootDir();
	// save the version file in $instanceId/version.json
	{
		QString version1 =  PathCombine(inst_dir, "/version.json");
		ensurePathExists(version1);
		QFile  vfile1 (version1);
		vfile1.open(QIODevice::Truncate | QIODevice::WriteOnly );
		vfile1.write(DlJob->m_data);
		vfile1.close();
	}
	
	// save the version file in versions/$version/$version.json
	/*
		//QString version2 =  QString("versions/") + version_id + "/" + version_id + ".json";
		//ensurePathExists(version2);
		//QFile  vfile2 (version2);
		//vfile2.open(QIODevice::Truncate | QIODevice::WriteOnly );
		//vfile2.write(DlJob->m_data);
		//vfile2.close();
	*/
	
	jarlibStart();
}

void OneSixUpdate::versionFileFailed()
{
	error("Failed to download the version description. Try again.");
	emitEnded();
}

void OneSixUpdate::jarlibStart()
{
	OneSixInstance * inst = (OneSixInstance *) m_inst;
	bool successful = inst->reloadFullVersion();
	if(!successful)
	{
		error("Failed to load the version description file (version.json). It might be corrupted, missing or simply too new.");
		emitEnded();
		return;
	}
	
	QSharedPointer<FullVersion> version = inst->getFullVersion();
	
	// download the right jar, save it in versions/$version/$version.jar
	QString urlstr("http://s3.amazonaws.com/Minecraft.Download/versions/");
	urlstr += version->id + "/" + version->id + ".jar";
	QString targetstr ("versions/");
	targetstr += version->id + "/" + version->id + ".jar";
	
	auto dljob = DownloadJob::create(QUrl(urlstr), targetstr);
	jarlibDownloadJob.reset(new JobList());
	jarlibDownloadJob->add(dljob);
	
	auto libs = version->getActiveNativeLibs();
	libs.append(version->getActiveNormalLibs());
	
	for(auto lib: libs)
	{
		QString download_path = lib->downloadPath();
		QString storage_path = "libraries/" + lib->storagePath();
		jarlibDownloadJob->add(DownloadJob::create(net_manager, download_path, storage_path));
	}
	connect(jarlibDownloadJob.data(), SIGNAL(finished()), SLOT(jarlibFinished()));
	connect(jarlibDownloadJob.data(), SIGNAL(failed()), SLOT(jarlibFailed()));
	connect(jarlibDownloadJob.data(), SIGNAL(progress(qint64,qint64)), SLOT(updateDownloadProgress(qint64,qint64)));

	download_queue.enqueue(jarlibDownloadJob);
}

void OneSixUpdate::jarlibFinished()
{
	emit gameUpdateComplete();
	emitEnded();
}

void OneSixUpdate::jarlibFailed()
{
	error("Failed to download the binary garbage. Try again. Maybe. IF YOU DARE");
	emitEnded();
}

void OneSixUpdate::error(const QString &msg)
{
	emit gameUpdateError(msg);
}

void OneSixUpdate::updateDownloadProgress(qint64 current, qint64 total)
{
	// The progress on the current file is current / total
	float currentDLProgress = (float) current / (float) total;
	setProgress((int)(currentDLProgress * 100)); // convert to percentage
}

