#include "linepicker.h"
#include <QLayout>
#include <QDebug>
#include <QColorDialog>

#include "global.h"

LinePicker::LinePicker(int index, bool bChecked, int line, QColor color, QWidget *parent)
    : QWidget(parent)
    , m_index(index)
{
    QHBoxLayout *pLayout = new QHBoxLayout();
    m_pCheckBox = new QCheckBox();
    if(bChecked)
        m_pCheckBox->setCheckState(Qt::Checked);
    else {
        m_pCheckBox->setCheckState(Qt::Unchecked);
    }

    m_pSpinBox = new QSpinBox();
    m_pSpinBox->setRange(1, SWIRHEIGHT);
    m_pSpinBox->setValue(line > 0 ? line : 1);
    m_pSpinBox->setFixedWidth(50);

    m_pColorButton = new QLabel();
    m_pColorButton->setStyleSheet(QString("background-color: %1").arg(color.name()));
    m_pColorButton->setFixedSize(16, 16);

    pLayout->addWidget(m_pCheckBox);
    pLayout->addWidget(m_pSpinBox);
    pLayout->addWidget(m_pColorButton);

    setLayout(pLayout);

    connect(m_pCheckBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
    connect(m_pSpinBox, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChanged(int)));
}

LinePicker::~LinePicker()
{
}

void LinePicker::checkBoxChanged(int state)
{
    if(state == Qt::Checked)
    {
        emit addLine(m_index, m_pSpinBox->value());
    }
    else {
        emit removeLine(m_index);
    }
}

void LinePicker::deselect()
{
    disconnect(m_pCheckBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
    m_pCheckBox->setCheckState(Qt::Unchecked);
    connect(m_pCheckBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
}

void LinePicker::spinBoxChanged(int value)
{
    deselect();
}

