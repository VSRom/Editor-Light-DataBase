#include "Create_Table.h"
//===========================================================================================================
Create_Table::Create_Table(QWidget* parent)
	: QDialog(parent) {
	setup_ui();
}
//===========================================================================================================
Create_Table::~Create_Table() {}
//===========================================================================================================
void Create_Table::setup_ui() {
	setWindowTitle("Создание таблицы");
	setMinimumSize(800, 600);

	QVBoxLayout* layout = new QVBoxLayout(this);

	// Поле имени таблицы
	QHBoxLayout* nameLayout = new QHBoxLayout();
	nameLayout->addWidget(new QLabel("Имя таблицы:"));
	tableName_ = new QLineEdit();
	tableName_->setPlaceholderText("Введите имя таблицы");
	nameLayout->addWidget(tableName_);
	layout->addLayout(nameLayout);

	// Основное пространство
	QScrollArea* scroll = new QScrollArea();
	scroll->setWidgetResizable(true);
	QWidget* scroll_content = new QWidget();
	col_layout_ = new QVBoxLayout(scroll_content);
	scroll->setWidget(scroll_content);

	layout->addWidget(scroll);

	add_col_ = new QPushButton("+ (Доб. столбец)", this);
	layout->addWidget(add_col_);

	connect(add_col_, &QPushButton::clicked, this, &Create_Table::add_col_row);

	setLayout(layout);
}
//===========================================================================================================
QString Create_Table::get_sql() const {

	return QString();
}
//===========================================================================================================
void Create_Table::add_col_row() {

}
//===========================================================================================================