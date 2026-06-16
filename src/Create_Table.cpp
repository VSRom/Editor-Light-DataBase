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
	setMinimumSize(1115, 800);

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

	scroll_content_ = new QWidget();
	col_layout_ = new QGridLayout(scroll_content_);
	col_layout_->setSpacing(15);
	col_layout_->setContentsMargins(15, 15, 15, 15);

	col_layout_->setAlignment(Qt::AlignTop | Qt::AlignLeft);

	scroll_content_->setMinimumWidth(800);
	scroll_content_->setMaximumHeight(1000);


	scroll->setWidget(scroll_content_);
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
	group->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	group->setMinimumWidth(200);
	group->setMaximumWidth(220);
	group->setFixedHeight(80);

	QComboBox* box = new QComboBox(group);
	box->setEditable(true);	// Редактируем бокс
	box->addItems(types_db_);

	QLineEdit* line = new QLineEdit(group);
	line->setPlaceholderText("Название");

	QPushButton* btn = new QPushButton("X", group); // Привязываем к очищаемому объекту внутри окна Создания таблицы(устранение утечки)
	btn->setFixedSize(20, 20);
	btn->setProperty("class", "del-btn");  // Применяем стиль из QSS
	btn->setText("X");
	btn->setToolTip("Удалить столбец");

	QGridLayout* grid = new QGridLayout(group);
	grid->setContentsMargins(6, 6, 6, 6);
	grid->setSpacing(4);

	grid->addWidget(line, 0, 0, 1, 1);
	grid->addWidget(box, 1, 0, 1, 1);
	grid->addWidget(btn, 0, 1, 2, 1, Qt::AlignCenter);

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

		col_layout_->removeWidget(group);
		col_row_.removeAt(rem_index);

		for (int i = 0; i < col_row_.size(); i++) {
			int row = i / 5;
			int col = i % 5;
			col_layout_->addWidget(col_row_[i].container_, row, col, Qt::AlignTop | Qt::AlignLeft);
		}

		group->hide();
		delete group;
				});


colrow.container_ = group;
colrow.typeCombo_ = box;
colrow.nameEdit_ = line;
colrow.removeBtn_ = btn;

int index = col_row_.size();
int row = index / 5;
int col = index % 5;

col_layout_->addWidget(group, row, col, Qt::AlignTop | Qt::AlignLeft);
col_row_.append(colrow);
}
//===========================================================================================================