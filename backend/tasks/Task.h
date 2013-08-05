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

#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QString>

#include "libmmc_config.h"

class LIBMULTIMC_EXPORT Task : public QObject
{
	Q_OBJECT
public:
	explicit Task(QObject *parent = 0);
	
	// Starts the task.
	void startTask();
	
	QString getStatus() const;
	int getProgress() const;
	
	bool isRunning() const;
	
	/*!
	 * \brief Calculates and sets the task's progress based on the number of parts completed out of the total number to complete.
	 * This is essentially just shorthand for setProgress((parts / whole) * 100);
	 * \param parts The parts out of the whole completed. This parameter should
	 * be less than whole. If it is greater than whole, progress is set to 100.
	 * \param whole The total number of things that need to be completed.
	 */
	void calcProgress(int parts, int whole);
	
protected slots:
	void setStatus(const QString& status);
	void setProgress(int progress);
	
signals:
	void started(Task* task);
	void ended(Task* task);
	
	void started();
	void ended();
	
	void statusChanged(Task* task, const QString& status);
	void progressChanged(Task* task, int progress);
	
	void statusChanged(const QString& status);
	void progressChanged(int progress);
	
protected:
	virtual void executeTask() = 0;
	
	virtual void emitStarted();
	virtual void emitEnded();
	
	virtual void emitStatusChange(const QString &status);
	virtual void emitProgressChange(int progress);
	
	QString status;
	int progress;
	bool running = false;
};

#endif // TASK_H
