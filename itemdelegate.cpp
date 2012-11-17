#include "debug.h"
#include "itemdelegate.h"
#include <QItemDelegate>
#include <QLineEdit>
#include <QValidator>
#include <QEvent>
#include <QKeyEvent>

ItemDelegate::ItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
  setObjectName("ItemDelegate");

  D_CONSTRUCT("")
}

ItemDelegate::~ItemDelegate()
{
  D_DESTRUCT("")
}

QWidget *ItemDelegate::createEditor(QWidget *parent,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
  QLineEdit*  editor    = new QLineEdit(parent);
  QValidator* validator = new QRegExpValidator(QRegExp("[0-9]{0,10}"), editor);
  editor->setFrame(false);
  editor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  editor->setValidator(validator);
  return editor;
}

void ItemDelegate::setEditorData(QWidget *editor,
                                 const QModelIndex &index) const
{
  return QStyledItemDelegate::setEditorData(editor, index);
}

void ItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
  return QStyledItemDelegate::setModelData(editor, model, index);
}

void ItemDelegate::updateEditorGeometry(QWidget *editor,
                                        const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  return QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

void ItemDelegate::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
  QStyledItemDelegate::paint(painter, option, index);
}
