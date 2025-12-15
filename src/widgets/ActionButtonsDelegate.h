#pragma once

#include <QApplication>
#include <QStyle>
#include <QStyledItemDelegate>

namespace Widgets {

class ActionButtonsDelegate : public QStyledItemDelegate {
    Q_OBJECT
   public:
    explicit ActionButtonsDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    void setHoveredRow(int row);

   signals:
    void addClicked(int row);
    void editClicked(int row);
    void deleteClicked(int row);

   private:
    int hoveredRow = -1;

    QRect addRect(const QStyleOptionViewItem& option) const;
    QRect editRect(const QStyleOptionViewItem& option) const;
    QRect deleteRect(const QStyleOptionViewItem& option) const;
};

}  // namespace Widgets
