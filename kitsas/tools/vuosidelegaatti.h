#ifndef VUOSIDELEGAATTI_H
#define VUOSIDELEGAATTI_H

#include <QItemDelegate>

class VuosiDelegaatti : public QItemDelegate
{
    Q_OBJECT
public:
    explicit VuosiDelegaatti(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

};

#endif // VUOSIDELEGAATTI_H
