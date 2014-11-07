/***********************************************************************
* Copyright 2003-2004  Max Howell <max.howell@methylblue.com>
* Copyright 2008-2009  Martin Sandsmark <martin.sandsmark@kde.org>
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

#include "settingsDialog.h"
#include "Config.h"

#include <QRadioButton>
#include <QCloseEvent>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QButtonGroup>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    QDialogButtonBox *buttons = new QDialogButtonBox;
    QPushButton *resetButton = buttons->addButton(QDialogButtonBox::Reset);
    QPushButton *closeButton = buttons->addButton(QDialogButtonBox::Close);
    layout()->addWidget(buttons);

    QVBoxLayout *vbox = new QVBoxLayout;
    colourSchemeGroup->setFlat(true);

    m_schemaGroup = new QButtonGroup;
    QRadioButton *radioButton;

    radioButton = new QRadioButton(tr("Rainbow"));
    vbox->addWidget(radioButton);
    m_schemaGroup->addButton(radioButton, Filelight::Rainbow);

    radioButton = new QRadioButton(tr("System colors"));
    vbox->addWidget(radioButton);
    m_schemaGroup->addButton(radioButton, Filelight::KDE);

    radioButton = new QRadioButton(tr("High contrast"));
    vbox->addWidget(radioButton);
    m_schemaGroup->addButton(radioButton, Filelight::HighContrast);

    colourSchemeGroup->setLayout(vbox);

    //read in settings before you make all those nasty connections!
    reset(); //makes dialog reflect global settings

    connect(&m_timer, SIGNAL(timeout()), SIGNAL(mapIsInvalid()));

    connect(m_addButton,    SIGNAL(clicked()), SLOT(addFolder()));
    connect(m_removeButton, SIGNAL(clicked()), SLOT(removeFolder()));
    connect(resetButton,  SIGNAL(clicked()), SLOT(reset()));
    connect(closeButton,  SIGNAL(clicked()), SLOT(close()));

    connect(colourSchemeGroup, SIGNAL(clicked(int)), SLOT(changeScheme(int)));
    connect(contrastSlider, SIGNAL(valueChanged(int)), SLOT(changeContrast(int)));
    connect(contrastSlider, SIGNAL(sliderReleased()), SLOT(slotSliderReleased()));

    connect(scanAcrossMounts,       SIGNAL(toggled(bool)), SLOT(startTimer()));
    connect(dontScanRemoteMounts,   SIGNAL(toggled(bool)), SLOT(startTimer()));
    connect(dontScanRemovableMedia, SIGNAL(toggled(bool)), SLOT(startTimer()));
    connect(scanAcrossMounts,       SIGNAL(toggled(bool)),
            SLOT(toggleScanAcrossMounts(bool)));
    connect(dontScanRemoteMounts,   SIGNAL(toggled(bool)),
            SLOT(toggleDontScanRemoteMounts(bool)));
    connect(dontScanRemovableMedia, SIGNAL(toggled(bool)),
            SLOT(toggleDontScanRemovableMedia(bool)));

    connect(useAntialiasing,    SIGNAL(toggled(bool)), SLOT(toggleUseAntialiasing(bool)));
    connect(varyLabelFontSizes, SIGNAL(toggled(bool)), SLOT(toggleVaryLabelFontSizes(bool)));
    connect(showSmallFiles,     SIGNAL(toggled(bool)), SLOT(toggleShowSmallFiles(bool)));

    connect(minFontPitch, SIGNAL (valueChanged(int)), SLOT(changeMinFontPitch(int)));

    m_addButton->setIcon(QIcon(QLatin1String("folder-open")));
    m_removeButton->setIcon(QIcon(QLatin1String("list-remove")));
}


void SettingsDialog::closeEvent(QCloseEvent*)
{
    //if an invalidation is pending, force it now!
    if (m_timer.isActive()) m_timer.setInterval(0);

    Config::write();

    deleteLater();
}


void SettingsDialog::reset()
{
    Config::read();

    //tab 1
    scanAcrossMounts->setChecked(Config::scanAcrossMounts);
    dontScanRemoteMounts->setChecked(!Config::scanRemoteMounts);
    dontScanRemovableMedia->setChecked(!Config::scanRemovableMedia);

    dontScanRemoteMounts->setEnabled(Config::scanAcrossMounts);
    //  dontScanRemovableMedia.setEnabled(Config::scanAcrossMounts);

    m_listBox->clear();
    m_listBox->addItems(Config::skipList);
    m_listBox->setCurrentRow(0);

    m_removeButton->setEnabled(m_listBox->count() > 0);

    //tab 2
    if (m_schemaGroup->checkedId() != Config::scheme) //TODO: This is probably wrong
    {
        m_schemaGroup->button(Config::scheme)->setChecked(true);
        //colourSchemeGroup->setSelected(Config::scheme);
        //setButton doesn't call a single QButtonGroup signal!
        //so we need to call this ourselves (and hence the detection above)
//        changeScheme(Config::scheme);
    }
    contrastSlider->setValue(Config::contrast);

    useAntialiasing->setChecked(Config::antialias);

    varyLabelFontSizes->setChecked(Config::varyLabelFontSizes);
    minFontPitchLabel->setEnabled(Config::varyLabelFontSizes);
    minFontPitch->setEnabled(Config::varyLabelFontSizes);
    minFontPitch->setValue(Config::minFontPitch);
    showSmallFiles->setChecked(Config::showSmallFiles);
}



void SettingsDialog::toggleScanAcrossMounts(bool b)
{
    Config::scanAcrossMounts = b;

    dontScanRemoteMounts->setEnabled(b);
    //dontScanRemovableMedia.setEnabled(b);
}

void SettingsDialog::toggleDontScanRemoteMounts(bool b)
{
    Config::scanRemoteMounts = !b;
}

void SettingsDialog::toggleDontScanRemovableMedia(bool b)
{
    Config::scanRemovableMedia = !b;
}



void SettingsDialog::addFolder()
{
    const QUrl url = QFileDialog::getExistingDirectory(this, tr("Select path to ignore"), QDir::rootPath());

    //TODO error handling!
    //TODO wrong protocol handling!

    if (!url.isEmpty())
    {
        const QString path = url.toLocalFile();

        if (!Config::skipList.contains(path))
        {
            Config::skipList.append(path);
            m_listBox->addItem(path);
            if (m_listBox->currentItem() == 0) m_listBox->setCurrentRow(0);
            m_removeButton->setEnabled(true);
        }
        else QMessageBox::information(this, tr("Folder already ignored"), tr("That folder is already set to be excluded from scans."));
    }
}


void SettingsDialog::removeFolder()
{
    Config::skipList.removeAll(m_listBox->currentItem()->text()); //removes all entries that match

    //safest method to ensure consistency
    m_listBox->clear();
    m_listBox->addItems(Config::skipList);

    m_removeButton->setEnabled(m_listBox->count() > 0);
    if (m_listBox->count() > 0) m_listBox->setCurrentRow(0);
}


void SettingsDialog::startTimer()
{
    m_timer.setSingleShot(true);
    m_timer.start(TIMEOUT);
}

void SettingsDialog::changeScheme(int s)
{
    Config::scheme = (Filelight::MapScheme)s;
    emit canvasIsDirty(1);
}
void SettingsDialog::changeContrast(int c)
{
    Config::contrast = c;
    emit canvasIsDirty(3);
}
void SettingsDialog::toggleUseAntialiasing(bool b)
{
    Config::antialias = b;
    emit canvasIsDirty(2);
}
void SettingsDialog::toggleVaryLabelFontSizes(bool b)
{
    Config::varyLabelFontSizes = b;
    minFontPitchLabel->setEnabled(b);
    minFontPitch->setEnabled(b);
    emit canvasIsDirty(0);
}
void SettingsDialog::changeMinFontPitch(int p)
{
    Config::minFontPitch = p;
    emit canvasIsDirty(0);
}
void SettingsDialog::toggleShowSmallFiles(bool b)
{
    Config::showSmallFiles = b;
    emit canvasIsDirty(1);
}


void SettingsDialog::slotSliderReleased()
{
    emit canvasIsDirty(2);
}


void SettingsDialog::reject()
{
    //called when escape is pressed
    reset();
    QDialog::reject();   //**** doesn't change back scheme so far
}


