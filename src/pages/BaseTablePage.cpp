#include "BaseTablePage.h"

#include <QHeaderView>
#include <QMouseEvent>

BaseTablePage::BaseTablePage(QWidget* parent)
    : QWidget(parent),
      table(new QTableView(this)),
      model(new QStandardItemModel(this)),
      actionsDelegate(new Widgets::ActionButtonsDelegate(this)) {
    setupTable();
    setupActionsDelegate();
    applyActionsDelegateToLastColumn();
}

void BaseTablePage::setupTable() {
    table->setModel(model);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    table->setMouseTracking(true);
    table->viewport()->installEventFilter(this);
    table->verticalHeader()->setVisible(false);
}

void BaseTablePage::setupActionsDelegate() {
    connect(actionsDelegate, &Widgets::ActionButtonsDelegate::addClicked, this, [this](int row) {
        selectRow(row);
        onRowAdd(row);
    });
    connect(actionsDelegate, &Widgets::ActionButtonsDelegate::editClicked, this, [this](int row) {
        selectRow(row);
        onRowEdit(row);
    });
    connect(actionsDelegate, &Widgets::ActionButtonsDelegate::deleteClicked, this, [this](int row) {
        selectRow(row);
        onRowDelete(row);
    });
}

void BaseTablePage::applyActionsDelegateToLastColumn() {
    // for (size_t i = 0; i < model->columnCount() - 2; i++) {
        // table->setItemDelegateForColumn(model->columnCount() - 2, new QStyledItemDelegate(this));
    // }

    table->setItemDelegateForColumn(model->columnCount() - 1, actionsDelegate);
    auto* hdr = table->horizontalHeader();
    hdr->setSectionResizeMode(QHeaderView::Stretch);
    hdr->setSectionResizeMode(model->columnCount() - 1, QHeaderView::ResizeToContents);
}

bool BaseTablePage::eventFilter(QObject* obj, QEvent* event) {
    if (obj == table->viewport()) {
        if (event->type() == QEvent::MouseMove) {
            const auto* me = static_cast<QMouseEvent*>(event);
            const QModelIndex idx = table->indexAt(me->pos());
            actionsDelegate->setHoveredRow(idx.isValid() ? idx.row() : -1);
            table->viewport()->update();
        } else if (event->type() == QEvent::Leave) {
            actionsDelegate->setHoveredRow(-1);
            table->viewport()->update();
        }
    }
    return QWidget::eventFilter(obj, event);
}

void BaseTablePage::selectRow(int row) {
    if (row < 0 || row >= model->rowCount()) return;
    table->selectRow(row);
}
