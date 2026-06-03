#pragma once
#include <QDialog>
#include <QPushButton>
//===========================================================================================================
class Create_Table : public QDialog {
	Q_OBJECT

public:
	explicit Create_Table(QWidget* parent = nullptr);
	~Create_Table();

	QString get_sql() const;

private:
	void setup_ui();
};
//===========================================================================================================