#include <QObject>
#include <QList>
#include <QPair>
#include <QString>
#include <QLineEdit>
#include <QHash>
#include <QVariant>
#include <QDialog>
#include "Table_Explorer.h"
//===========================================================================================================
class Add_Row_Dialog : public QDialog{
	Q_OBJECT

public:
	explicit Add_Row_Dialog(QList<Table_Explorer::ColumnInfo> colInfo, QWidget* parent = nullptr);

	void setup_ui();
	QHash<QString, QVariant> getValues();

private:
	QList<Table_Explorer::ColumnInfo> m_colInfo;
	QList<QPair<QString, QLineEdit *>> m_fields;
};
//===========================================================================================================