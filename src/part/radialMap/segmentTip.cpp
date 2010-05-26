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

#include "segmentTip.h"

#include "part/fileTree.h"
#include "part/Config.h"

#include <KApplication>    //installing eventFilters
#include <KGlobal>
#include <KLocale>
#include <KWindowSystem>

#include <QtGui/QPainter>
#include <QtGui/QToolTip>

#include <cstdlib>

namespace RadialMap {

SegmentTip::SegmentTip(uint h)
        : QWidget(0, Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint)
        , m_cursorHeight(-h)
{
#if QT_VERSION >= 0x040500
    setAttribute(Qt::WA_TranslucentBackground);
#endif
}

void
SegmentTip::moveTo(QPoint p, bool placeAbove)
{
    //**** this function is very slow and seems to be visibly influenced by operations like mapFromGlobal() (who knows why!)
    //  ** so any improvements are much desired

    //TODO uints could improve the class
    p.rx() -= rect().center().x();
    p.ry() -= (placeAbove ? 8 + height() : m_cursorHeight - 8);

    const int x  = p.x();
    const int y  = p.y();

    move(x, y);
    show();
}

void
SegmentTip::updateTip(const File* const file, const Folder* const root)
{
    const QString s1  = file->fullPath(root);
    QString s2        = file->humanReadableSize();
    KLocale *loc      = KGlobal::locale();
    const uint MARGIN = 3;
    const uint pc     = 100 * file->size() / root->size();
    uint maxw         = 0;
    uint h            = fontMetrics().height()*2 + 2*MARGIN;

    if (pc > 0) s2 += QString(" (%1%)").arg(loc->formatNumber(pc, 0));

    m_text  = s1;
    m_text += '\n';
    m_text += s2;

    if (file->isFolder())
    {
        int files  = static_cast<const Folder*>(file)->children();
        const uint pc = uint((100 * files) / (double)root->children());
        QString s3    = i18np("File: %1", "Files: %1", files);

        if (pc > 0) s3 += QString(" (%1%)").arg(loc->formatNumber(pc, 0));

        maxw    = fontMetrics().width(s3);
        h      += fontMetrics().height();
        m_text += '\n';
        m_text += s3;
    }

    uint
    w = fontMetrics().width(s1);
    if (w > maxw) maxw = w;
    w = fontMetrics().width(s2);
    if (w > maxw) maxw = w;

    resize(maxw + 2 * MARGIN, h);

    // Paint
    m_pixmap = QPixmap(size());

    QColor bg = QToolTip::palette().color(QPalette::Active, QPalette::Background);
    const QColor fg = QToolTip::palette().color(QPalette::Active, QPalette::WindowText);

    if (KWindowSystem::compositingActive())
        bg.setAlpha(200);

    m_pixmap.fill(bg);

    QPainter paint(&m_pixmap);
    if (Config::antialias) paint.setRenderHint(QPainter::Antialiasing);

    paint.setPen(fg);
    paint.drawRect(rect());
    paint.drawText(rect(), Qt::AlignCenter, m_text);

    repaint(); //Bug in Qt? (If we don't change the size of the pixmap, and don't do this, we aren't redrawn)
}

bool
SegmentTip::eventFilter(QObject*, QEvent *e)
{
    switch (e->type())
    {
    case QEvent::Leave:
    case QEvent::FocusOut:
    case QEvent::Wheel:
        hide(); //FALL THROUGH
    case QEvent::FocusIn:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    default:
        return false; //allow this event to passed to target
    }
}

void
SegmentTip::paintEvent(QPaintEvent*)
{
    QPainter(this).drawPixmap(0, 0, m_pixmap);
}

void
SegmentTip::hideEvent(QHideEvent*)
{
    kapp->removeEventFilter(this);
}

void
SegmentTip::showEvent(QShowEvent*)
{
    kapp->installEventFilter(this);
}

} //namespace RadialMap

