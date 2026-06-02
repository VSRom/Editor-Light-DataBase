#pragma once
#include <QDialog>
#include <QString>
#include <QPushButton>
//===========================================================================================================
class Create_Table : public QDialog {
	Q_OBJECT

public:
	explicit Create_Table(QWidget* parent = nullptr);
	~Create_Table();

	QString tab_create() const;

private:
	void setup_ui();
};
//===========================================================================================================