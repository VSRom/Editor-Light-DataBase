#include "Add_Row_Dialog.h"
#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
//===========================================================================================================
Add_Row_Dialog::Add_Row_Dialog(QList<Table_Explorer::ColumnInfo> colInfo, QWidget *parent)
	: QDialog(parent), m_colInfo(colInfo)
{
	setup_ui();
}
//===========================================================================================================
void Add_Row_Dialog::setup_ui() {
	QFormLayout *formLayout = new QFormLayout(this);

	for (const auto& col : m_colInfo) {
		if (col.isPrimaryKey) continue;

		QString labelText = QString("%1 (%2):").arg(col.name).arg(col.type);
		QLineEdit *lineEdit = new QLineEdit(this);
		formLayout->addRow(labelText, lineEdit);

		m_fields.append({col.name, lineEdit});
	}

	QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	formLayout->addRow(btnBox);
}
//===========================================================================================================
QHash<QString, QVariant> Add_Row_Dialog::getValues() {
	QHash<QString, QVariant> tempHash;

	for (const auto& fiel : m_fields) {
		QString text = fiel.second->text().trimmed();
		if (text.isEmpty())
			tempHash.insert(fiel.first, QVariant());
		else
			tempHash.insert(fiel.first, text);
	}

	return tempHash;
}
//===========================================================================================================
