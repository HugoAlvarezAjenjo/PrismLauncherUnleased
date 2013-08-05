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

#include "include/pathutils.h"

#include <QFileInfo>
#include <QDir>

QString PathCombine(QString path1, QString path2)
{
	if (!path1.endsWith('/'))
		return path1.append('/').append(path2);
	else
		return path1.append(path2);
}

QString PathCombine(QString path1, QString path2, QString path3)
{
	return PathCombine(PathCombine(path1, path2), path3);
}

QString AbsolutePath(QString path)
{
	return QFileInfo(path).absolutePath();
}

QString badFilenameChars = "\"\\/?<>:*|!";

QString RemoveInvalidFilenameChars(QString string, QChar replaceWith)
{
	for (int i = 0; i < string.length(); i++)
	{
		if (badFilenameChars.contains(string[i]))
		{
			string[i] = replaceWith;
		}
	}
	return string;
}

QString DirNameFromString(QString string, QString inDir)
{
	int num = 0;
	QString dirName = RemoveInvalidFilenameChars(string, '-');
	while (QFileInfo(PathCombine(inDir, dirName)).exists())
	{
		num++;
		dirName = RemoveInvalidFilenameChars(dirName, '-') + QString::number(num);
		
		// If it's over 9000
		if (num > 9000)
			return "";
	}
	return dirName;
}

bool ensurePathExists(QString filenamepath)
{
	QFileInfo a ( filenamepath );
	QDir dir;
	return (dir.mkpath ( a.path() ));
}

