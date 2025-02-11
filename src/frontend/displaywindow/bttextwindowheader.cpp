/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, https://bibletime.info/
*
* Copyright 1999-2021 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License
* version 2.0.
*
**********/

#include "bttextwindowheader.h"

#include <QHBoxLayout>
#include <QLayout>
#include <QSizePolicy>
#include <QStringList>
#include <Qt>
#include <QWidget>
#include "../../backend/drivers/cswordmoduleinfo.h"
#include "../../util/btconnect.h"
#include "../../util/btmodules.h"
#include "bttextwindowheaderwidget.h"
#include "cdisplaywindow.h"


BtTextWindowHeader::BtTextWindowHeader(CSwordModuleInfo::ModuleType modtype,
                                       QStringList modules,
                                       CDisplayWindow *window)
        : QWidget(window),
        BtWindowModuleChooser(modtype, window)
{
    QHBoxLayout* layout = new QHBoxLayout ( this );
    layout->setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setLayoutDirection(Qt::LeftToRight);
    setModules(modules);
    BT_CONNECT(window, &CDisplayWindow::sigModuleListSet,
               this /* Needed */, [this] { slotBackendModulesChanged(); });
    BT_CONNECT(window, &CDisplayWindow::sigModuleListChanged,
               this, &BtTextWindowHeader::slotWindowModulesChanged);
}

void BtTextWindowHeader::slotBackendModulesChanged() {
    m_modules = m_window->getModuleList();

    adjustWidgetCount();

    int const leftLikeModules = leftLikeParallelModules(m_modules);
    for (int i = 0; i < m_widgetList.count(); i++)
        m_widgetList.at(i)->updateWidget(m_modules,
                                         m_modules.at(i),
                                         i,
                                         leftLikeModules);
}

void BtTextWindowHeader::slotWindowModulesChanged() {
    m_modules = m_window->getModuleList();
    adjustWidgetCount();
    updateWidgets();
}

void BtTextWindowHeader::adjustWidgetCount(bool adjustToZero) {
    int widgetCountDifference = 0;
    if (adjustToZero) {
        widgetCountDifference = m_widgetList.count();
    }
    else {
        widgetCountDifference = m_widgetList.count() - (m_modules.count());
    }
    if (m_moduleType == CSwordModuleInfo::GenericBook && !adjustToZero) {
        widgetCountDifference = (1 - m_widgetList.count()) * -1;
    }
    //if there are more buttons than modules, delete buttons
    if (widgetCountDifference > 0) {
        while (widgetCountDifference) {
            // it should be safe to delete the button later
            BtTextWindowHeaderWidget* w = m_widgetList.takeFirst();
            w->setParent(nullptr);
            w->deleteLater();
            widgetCountDifference--;
        }
    }
    // if there are more modules than buttons, add buttons
    if (widgetCountDifference < 0) {
        while (widgetCountDifference) {
            addWidget();
            widgetCountDifference++;
        }
    }
}

BtTextWindowHeaderWidget* BtTextWindowHeader::addWidget() {
    BtTextWindowHeaderWidget * const w =
            new BtTextWindowHeaderWidget(m_moduleType, this);
    layout()->addWidget(w);
    m_widgetList.append(w);

    // the button sends signals directly to the window which then signals back when the module
    // list has changed
    BT_CONNECT(w, &BtTextWindowHeaderWidget::sigModuleAdd,
               m_window, &CDisplayWindow::slotAddModule);
    BT_CONNECT(w, &BtTextWindowHeaderWidget::sigModuleReplace,
               m_window, &CDisplayWindow::slotReplaceModule);
    BT_CONNECT(w, &BtTextWindowHeaderWidget::sigModuleRemove,
               m_window, &CDisplayWindow::slotRemoveModule);

    return w;
}

void BtTextWindowHeader::setModules( QStringList useModules ) {
    m_modules = useModules;
    adjustWidgetCount(true);

    //if (!useModules.count()) return;
    for (int i = 0; i < useModules.count(); i++) {
        addWidget();
    }
    updateWidgets();
}

void BtTextWindowHeader::updateWidgets() {
    int const leftLikeModules = leftLikeParallelModules(m_modules);
    for (int i = 0; i < m_widgetList.count(); i++)
        m_widgetList.at(i)->updateWidget(m_modules,
                                         m_modules.at(i),
                                         i,
                                         leftLikeModules);
}
