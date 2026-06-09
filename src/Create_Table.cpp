#include "Create_Table.h"
//===========================================================================================================
Create_Table::Create_Table(QWidget* parent, const QStringList& types)
	: QDialog(parent), types_db_(types) {
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

	add_col_row();
}
//===========================================================================================================
QString Create_Table::get_sql() const {	// Сборка запроса для создания таблицы
	QString name_row = {};
	QString type_row = {};
	QStringList temp_sql_get = {};


	QString nameTab = tableName_->text().trimmed();	// Получили текст из строки имени таблицы
	if (!nameTab.isEmpty()) {
		for (int i = 0; i < col_row_.size(); i++) {
			name_row = col_row_[i].nameEdit_->text().trimmed();
			type_row = col_row_[i].typeCombo_->currentText();

			if (!name_row.isEmpty())
				temp_sql_get.append(QString("\"%1\" %2").arg(name_row, type_row));
		}
	}

	if (temp_sql_get.isEmpty())
		return QString();

	return QString("CREATE TABLE IF NOT EXISTS \"%1\" (%2);").arg(nameTab, temp_sql_get.join(", "));
}
//===========================================================================================================
void Create_Table::add_col_row() {
	Col_Row colrow;

	QHBoxLayout* layout = new QHBoxLayout();
	QComboBox* box = new QComboBox();
	QLineEdit* line = new QLineEdit();
	QPushButton* btn = new QPushButton("X", this);

	colrow.layout_ = layout;
	colrow.typeCombo_ = box;
	colrow.nameEdit_ = line;
	colrow.removeBtn_ = btn;

	colrow.typeCombo_->setEditable(true);	// Редактируем бокс
	colrow.typeCombo_->addItems(types_db_);
	
	colrow.nameEdit_->setPlaceholderText("Название");

	colrow.removeBtn_->setFixedWidth(30);

	connect(colrow.removeBtn_, &QPushButton::clicked, this, [this, layout, box, line, btn]() {		// Лямбда-удаление строки, с защитой
			if (col_row_.size() <= 1) {
				QMessageBox::warning(this, "Внимание", "Нельзя удалить последнюю строку!");
				return;
			}
			else {
				delete box; delete line; delete btn;
				delete layout;

				for (int i = 0; i < col_row_.size(); i++)
					if (col_row_[i].layout_ == layout) {	// Удаляем структуру
						col_row_.removeAt(i);
						break;
					}
			}});

	layout->addWidget(line);
	layout->addWidget(box);
	layout->addWidget(btn);
	
	col_layout_->addLayout(layout);

	col_row_.append(colrow);
//===========================================================================================================