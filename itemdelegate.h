#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QStyledItemDelegate>

class ItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit ItemDelegate(QObject *parent = 0);
  ~ItemDelegate();

  QWidget *createEditor(QWidget *parent,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;
  void setEditorData(QWidget *editor,
                     const QModelIndex &index) const;
  void setModelData(QWidget *editor,
                    QAbstractItemModel *model,
                    const QModelIndex &index) const;
  void updateEditorGeometry(QWidget *editor,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const;
  void paint(QPainter *painter,
             const QStyleOptionViewItem &option,
             const QModelIndex &index) const;

};

#endif // ITEMDELEGATE_H
