/***********************************************************************
* Copyright 2003-2004  Max Howell <max.howell@methylblue.com>
* Copyright 2008-2009  Martin Sandsmark <sandsmark@samfundet.no>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#include "define.h"
#include "mainWindow.h"

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KLocale>
#include <KUrl>

static KAboutData about(
    APP_NAME,
    0,
    ki18n("Filelight"),
    APP_VERSION,
    ki18n("Graphical disk-usage information"),
    KAboutData::License_GPL_V3,
    ki18n("(C) 2006 Max Howell\n(C) 2008, 2009 Martin Sandsmark"),
    KLocalizedString(),
    "http://www.methylblue.com/filelight/");


int main(int argc, char *argv[])
{
    using Filelight::MainWindow;

    about.addAuthor(ki18n("Martin Sandsmark"), ki18n("Maintainer"), "sandsmark@iskrembilen.com", "http://iskrembilen.com/");
    about.addAuthor(ki18n("Max Howell"),       ki18n("Original author"), "max.howell@methylblue.com", "http://www.methylblue.com/");
    about.addCredit(ki18n("Lukas Appelhans"),  ki18n("Help and support"));
    about.addCredit(ki18n("Steffen Gerlach"),  ki18n("Inspiration"), 0, "http://www.steffengerlach.de/");
    about.addCredit(ki18n("Mike Diehl"),       ki18n("Original documentation"), 0, 0);

    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    KLocale *tmpLocale = new KLocale(QLatin1String("filelight"));
    options.add(ki18nc("Path in the file system to scan", "+[path]").toString(tmpLocale).toLocal8Bit(), ki18n("Scan 'path'"));
    delete tmpLocale;
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    if (!app.isSessionRestored()) {
        MainWindow *mw = new MainWindow();

        KCmdLineArgs* const args = KCmdLineArgs::parsedArgs();
        if (args->count() > 0) mw->scan(args->url(0));
        args->clear();

        mw->show();
    }
    else RESTORE(MainWindow);

    return app.exec();
}
