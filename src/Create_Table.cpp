#include "Create_Table.h"
//===========================================================================================================
Create_Table::Create_Table(const QStringList& types, QWidget* parent)
	: types_db_(types), QDialog(parent) {
	setup_ui();
}
Create_Table::~Create_Table() {}
//===========================================================================================================
void Create_Table::setup_ui() {
	setWindowTitle("Создание таблицы");
	setMinimumSize(1150, 800);

	QVBoxLayout* layout = new QVBoxLayout(this);

	// Поле имени таблицы
	QHBoxLayout* nameLayout = new QHBoxLayout();
	nameLayout->addWidget(new QLabel("Имя таблицы:"));
	tableName_ = new QLineEdit();
	tableName_->setPlaceholderText("Поле ввода");
	nameLayout->addWidget(tableName_);
	layout->addLayout(nameLayout);

	// Основное пространство
	QScrollArea* scroll = new QScrollArea();
	scroll->setWidgetResizable(true);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	QWidget* scroll_content = new QWidget();
	col_layout_ = new QGridLayout(scroll_content);
	col_layout_->setSpacing(10);

	scroll_content->setMinimumWidth(400);
	scroll_content->setMinimumHeight(200);

	scroll->setWidget(scroll_content);
	layout->addWidget(scroll);

	add_col_ = new QPushButton("+ (Доб. столбец)", this);
	layout->addWidget(add_col_);

	connect(add_col_, &QPushButton::clicked, this, &Create_Table::add_col_row);

	// OK - CANCEL
	QHBoxLayout* btns_ok_canc = new QHBoxLayout();
	
	btn_ok_ = new QPushButton("Ok", this);
	btn_canc_ = new QPushButton("Cancel", this);
	btns_ok_canc->addStretch();					// Прижмёт вправо
	btns_ok_canc->addWidget(btn_ok_);
	btns_ok_canc->addWidget(btn_canc_);

	connect(btn_ok_, &QPushButton::clicked, this, &QDialog::accept);
	connect(btn_canc_, &QPushButton::clicked, this, &QDialog::reject);

	layout->addLayout(btns_ok_canc);
	setLayout(layout);

	add_col_row();

	col_layout_->setRowStretch(10, 1);
	col_layout_->setColumnStretch(5, 1);
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

	QGroupBox* group = new QGroupBox();
	group->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	group->setMinimumWidth(110);
	group->setMinimumHeight(60);

	QComboBox* box = new QComboBox();
	box->setEditable(true);	// Редактируем бокс
	box->addItems(types_db_);
	box->setMinimumWidth(80);


	QLineEdit* line = new QLineEdit();
	line->setPlaceholderText("Название");
	line->setMinimumWidth(90);

	QPushButton* btn = new QPushButton("X", this);
	btn->setFixedSize(25, 20);
	btn->setProperty("class", "del-btn");  // Применяем стиль из QSS

	btn->setText("X");

	btn->setToolTip("Удалить столбец");

	QGridLayout* grid = new QGridLayout(group);
	grid->addWidget(line, 0, 0);
	grid->addWidget(box, 1, 0);
	grid->addWidget(btn, 0, 2, 2, 1);

	// Лямбда-удаление строки, с защитой
	connect(btn, &QPushButton::clicked, this, [this, group]() {
		if (col_row_.size() <= 1) {
			QMessageBox::warning(this, "Внимание", "Нельзя удалить последнюю строку!");
			return;
		}

		int rem_index = -1;
		for (int i = 0; i < col_row_.size(); i++) {
			if (col_row_[i].container_ == group) {	// Удаляем структуру
				rem_index = i;
				break;
			}
		}
		if (rem_index == -1) return;

		col_row_.removeAt(rem_index);

		while (col_layout_->count() > 0) {
			QLayoutItem* item = col_layout_->takeAt(0);
			delete item;
		}

		for (int i = 0; i < col_row_.size(); i++) {
			int row = i / 6;
			int col = i % 6;
			col_layout_->addWidget(col_row_[i].container_, row, col, Qt::AlignTop | Qt::AlignLeft);
		}
		group->deleteLater();
	});

		colrow.container_ = group;
		colrow.typeCombo_ = box;
		colrow.nameEdit_ = line;
		colrow.removeBtn_ = btn;

		int index = col_row_.size();
		int row = index / 6;
		int col = index % 6;

		col_layout_->addWidget(group, row, col, Qt::AlignTop | Qt::AlignLeft);
		col_row_.append(colrow);
}
//===========================================================================================================