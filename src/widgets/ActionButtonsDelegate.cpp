#include "ActionButtonsDelegate.h"
#include <QPainter>
#include <QMouseEvent>

namespace Widgets {

ActionButtonsDelegate::ActionButtonsDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{
}

void ActionButtonsDelegate::setHoveredRow(int row)
{
	if (hoveredRow == row) return;
	hoveredRow = row;
}

QRect ActionButtonsDelegate::deleteRect(const QStyleOptionViewItem &option) const
{
	const int m = 6, s = 18, gap = 6;
	// Left aligned: third icon
	const int x = option.rect.left() + m + 2*(s + gap);
	return QRect(x, option.rect.center().y() - s/2, s, s);
}

QRect ActionButtonsDelegate::editRect(const QStyleOptionViewItem &option) const
{
	const int m = 6, s = 18, gap = 6;
	// Left aligned: second icon
	const int x = option.rect.left() + m + (s + gap);
	return QRect(x, option.rect.center().y() - s/2, s, s);
}

QRect ActionButtonsDelegate::addRect(const QStyleOptionViewItem &option) const
{
	const int m = 6, s = 18, gap = 6;
	// Left aligned: first icon
	const int x = option.rect.left() + m;
	return QRect(x, option.rect.center().y() - s/2, s, s);
}

void ActionButtonsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyledItemDelegate::paint(painter, option, index);
	if (index.row() != hoveredRow) return;

	static const QIcon addIcon(":/icons/icons/add.svg");
	static const QIcon editIcon(":/icons/icons/edit.svg");
	static const QIcon deleteIcon(":/icons/icons/delete.svg");

	addIcon.paint(painter, addRect(option));
	editIcon.paint(painter, editRect(option));
	deleteIcon.paint(painter, deleteRect(option));
}

bool ActionButtonsDelegate::editorEvent(QEvent *event, QAbstractItemModel *, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	if (index.row() != hoveredRow) return false;
	if (event->type() == QEvent::MouseButtonRelease) {
		auto *me = static_cast<QMouseEvent*>(event);
		if (deleteRect(option).contains(me->pos())) { emit deleteClicked(index.row()); return true; }
		if (editRect(option).contains(me->pos())) { emit editClicked(index.row()); return true; }
		if (addRect(option).contains(me->pos())) { emit addClicked(index.row()); return true; }
	}
	return false;
}

} // namespace Widgets

#include "moc_ActionButtonsDelegate.cpp"


