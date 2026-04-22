/* Copyright (c) 2020-2026 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "xhashwidget.h"

#include "ui_xhashwidget.h"

XHashWidget::XHashWidget(QWidget *pParent) : XShortcutsWidget(pParent), ui(new Ui::XHashWidget), m_pDevice(nullptr), m_nOffset(0), m_nSize(0), m_hashData()
{
    ui->setupUi(this);

    XOptions::adjustToolButton(ui->toolButtonReload, XOptions::ICONTYPE_RELOAD);
    XOptions::adjustToolButton(ui->toolButtonSave, XOptions::ICONTYPE_SAVE);

    ui->comboBoxType->setToolTip(tr("Type"));
    ui->comboBoxMethod->setToolTip(tr("Method"));
    ui->comboBoxMapMode->setToolTip(tr("Mode"));
    ui->lineEditOffset->setToolTip(tr("Offset"));
    ui->lineEditSize->setToolTip(tr("Size"));
    ui->lineEditHash->setToolTip(tr("Hash"));
    ui->tableViewRegions->setToolTip(tr("Regions"));
    ui->toolButtonReload->setToolTip(tr("Reload"));
    ui->toolButtonSave->setToolTip(tr("Save"));

    ui->lineEditHash->setValidatorMode(XLineEditValidator::MODE_TEXT);

    populateHashMethods();
}

XHashWidget::~XHashWidget()
{
    delete ui;
}

void XHashWidget::clearResults()
{
    ui->lineEditHash->clear();
    ui->tableViewRegions->setCustomModel(new QStandardItemModel(0, 0), true);
}

void XHashWidget::populateHashMethods()
{
    const bool bBlocked = ui->comboBoxMethod->blockSignals(true);
    ui->comboBoxMethod->clear();

    QList<XBinary::HASH> listHashMethods = XBinary::getHashMethodsAsList();
    qint32 nNumberOfMethods = listHashMethods.count();

    for (qint32 i = 0; i < nNumberOfMethods; i++) {
        XBinary::HASH hash = listHashMethods.at(i);
        ui->comboBoxMethod->addItem(XBinary::hashIdToString(hash), hash);
    }

    if (nNumberOfMethods > 1) {
        ui->comboBoxMethod->setCurrentIndex(1);
    } else if (nNumberOfMethods == 1) {
        ui->comboBoxMethod->setCurrentIndex(0);
    }

    ui->comboBoxMethod->blockSignals(bBlocked);
}

void XHashWidget::applyTableHeaders(QStandardItemModel *pModel)
{
    pModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
    pModel->setHeaderData(1, Qt::Horizontal, tr("Offset"));
    pModel->setHeaderData(2, Qt::Horizontal, tr("Size"));
    pModel->setHeaderData(3, Qt::Horizontal, tr("Hash"));
}

void XHashWidget::applyColumnWidths()
{
    qint32 nColumnSize = XLineEditHEX::getWidthFromMode(this, m_hashData.mode);

    ui->tableViewRegions->setColumnWidth(0, COLUMN_NAME_WIDTH);
    ui->tableViewRegions->setColumnWidth(1, nColumnSize);
    ui->tableViewRegions->setColumnWidth(2, nColumnSize);
    ui->tableViewRegions->setColumnWidth(3, COLUMN_FIXED_WIDTH);
}

void XHashWidget::fillRegionsModel()
{
    qint32 nNumberOfMemoryRecords = m_hashData.listMemoryRecords.count();
    QStandardItemModel *pModel = new QStandardItemModel(nNumberOfMemoryRecords, 4, ui->tableViewRegions);

    applyTableHeaders(pModel);

    for (qint32 i = 0; i < nNumberOfMemoryRecords; i++) {
        const HashProcess::MEMORY_RECORD &record = m_hashData.listMemoryRecords.at(i);

        QStandardItem *pItemName = new QStandardItem(record.sName);
        pModel->setItem(i, 0, pItemName);

        if (record.nOffset != -1) {
            QStandardItem *pItemOffset = new QStandardItem(XLineEditHEX::getFormatString(m_hashData.mode, record.nOffset));
            pModel->setItem(i, 1, pItemOffset);
        }

        if (record.nSize != -1) {
            QStandardItem *pItemSize = new QStandardItem(XLineEditHEX::getFormatString(m_hashData.mode, record.nSize));
            pModel->setItem(i, 2, pItemSize);
        }

        QStandardItem *pItemHash = new QStandardItem(record.sHash);
        pModel->setItem(i, 3, pItemHash);
    }

    XOptions::setModelTextAlignment(pModel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    XOptions::setModelTextAlignment(pModel, 1, Qt::AlignRight | Qt::AlignVCenter);
    XOptions::setModelTextAlignment(pModel, 2, Qt::AlignRight | Qt::AlignVCenter);
    XOptions::setModelTextAlignment(pModel, 3, Qt::AlignLeft | Qt::AlignVCenter);

    ui->tableViewRegions->setCustomModel(pModel, true);
    ui->tableViewRegions->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->tableViewRegions->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->tableViewRegions->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    ui->tableViewRegions->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    applyColumnWidths();
}

void XHashWidget::setData(QIODevice *pDevice, XBinary::FT fileType, qint64 nOffset, qint64 nSize, bool bAuto)
{
    m_pDevice = pDevice;
    m_nOffset = nOffset;
    m_nSize = nSize;

    if ((m_pDevice == nullptr) || (m_nOffset < 0)) {
        clearResults();
        return;
    }

    if (m_nSize == -1) {
        m_nSize = qMax<qint64>(0, m_pDevice->size() - m_nOffset);
    }

    if ((m_nSize < 0) || (m_nOffset + m_nSize > m_pDevice->size())) {
        m_nSize = qMax<qint64>(0, m_pDevice->size() - m_nOffset);
    }

    ui->lineEditOffset->setValue32_64(m_nOffset);
    ui->lineEditSize->setValue32_64(m_nSize);

    SubDevice subDevice(m_pDevice, m_nOffset, m_nSize);

    if (subDevice.open(QIODevice::ReadOnly)) {
        XFormats::setFileTypeComboBox(fileType, &subDevice, ui->comboBoxType);
        XFormats::getMapModesList(fileType, ui->comboBoxMapMode);
        subDevice.close();
    }

    if (bAuto) {
        reload();
    }
}

void XHashWidget::reload()
{
    if ((m_pDevice == nullptr) || (m_nSize <= 0) || (m_nOffset < 0)) {
        clearResults();
        return;
    }

    m_hashData.hash = static_cast<XBinary::HASH>(ui->comboBoxMethod->currentData().toInt());
    m_hashData.fileType = static_cast<XBinary::FT>(ui->comboBoxType->currentData().toInt());
    m_hashData.mapMode = static_cast<XBinary::MAPMODE>(ui->comboBoxMapMode->currentData().toInt());
    m_hashData.nOffset = m_nOffset;
    m_hashData.nSize = m_nSize;

    HashProcess hashProcess;
    XDialogProcess dhp(XOptions::getMainWidget(this), &hashProcess);
    dhp.setGlobal(getShortcuts(), getGlobalOptions());
    hashProcess.setData(m_pDevice, &m_hashData, dhp.getPdStruct());
    dhp.start();
    dhp.showDialogDelay();

    if (!dhp.isSuccess()) {
        clearResults();
        return;
    }

    ui->lineEditHash->setValue_String(m_hashData.sHash);

    fillRegionsModel();
}

void XHashWidget::adjustView()
{
}

void XHashWidget::reloadData(bool bSaveSelection)
{
    Q_UNUSED(bSaveSelection)
    reload();
}

void XHashWidget::on_toolButtonReload_clicked()
{
    reload();
}

void XHashWidget::on_comboBoxType_currentIndexChanged(int nIndex)
{
    Q_UNUSED(nIndex)

    XBinary::FT fileType = (XBinary::FT)(ui->comboBoxType->currentData().toInt());
    XFormats::getMapModesList(fileType, ui->comboBoxMapMode);

    reload();
}

void XHashWidget::on_comboBoxMethod_currentIndexChanged(int nIndex)
{
    Q_UNUSED(nIndex)

    reload();
}

void XHashWidget::registerShortcuts(bool bState)
{
    Q_UNUSED(bState)
    // TODO !!!
}

void XHashWidget::on_toolButtonSave_clicked()
{
    XShortcutsWidget::saveTableModel(ui->tableViewRegions->getProxyModel(), XBinary::getResultFileName(m_pDevice, QString("%1.txt").arg(tr("Hash"))));
}

void XHashWidget::on_tableViewRegions_customContextMenuRequested(const QPoint &pos)
{
    qint32 nRow = ui->tableViewRegions->currentIndex().row();

    if (nRow != -1) {
        QMenu contextMenu(this);

        QList<XShortcuts::MENUITEM> listMenuItems;

        getShortcuts()->_addMenuItem_CopyRow(&listMenuItems, ui->tableViewRegions);

        getShortcuts()->adjustContextMenu(&contextMenu, &listMenuItems);

        contextMenu.exec(ui->tableViewRegions->viewport()->mapToGlobal(pos));
    }
}

void XHashWidget::on_comboBoxMapMode_currentIndexChanged(int nIndex)
{
    Q_UNUSED(nIndex)

    reload();
}
