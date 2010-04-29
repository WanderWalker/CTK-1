/*=========================================================================

  Library:   CTK
 
  Copyright (c) 2010  Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.commontk.org/LICENSE

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 
=========================================================================*/

// Qt includes
#include <QApplication>
#include <QCleanlooksStyle>
#include <QDebug>
#include <QDesktopWidget>
#include <QLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QToolBar>

// CTK includes
#include "ctkMenuButton.h"

//-----------------------------------------------------------------------------
class ctkMenuButtonPrivate : public ctkPrivate<ctkMenuButton>
{
public:
  CTK_DECLARE_PUBLIC(ctkMenuButton);
  ctkMenuButtonPrivate();

  QRect indicatorRect() const;
  bool ShowMenu;
};

//-----------------------------------------------------------------------------
ctkMenuButtonPrivate::ctkMenuButtonPrivate()
{
  this->ShowMenu = false;
}

//-----------------------------------------------------------------------------
QRect ctkMenuButtonPrivate::indicatorRect()const
{
  CTK_P(const ctkMenuButton);
  
  QStyleOptionButton option;
  p->initStyleOption(&option);

  QRect downArrowRect = p->style()->visualRect(option.direction, option.rect, option.rect);
  downArrowRect.setRect(downArrowRect.right() - 13, downArrowRect.top(),
                        14, downArrowRect.height());
  downArrowRect = p->style()->visualRect(option.direction, option.rect, downArrowRect);
  return downArrowRect;
}

//-----------------------------------------------------------------------------
ctkMenuButton::ctkMenuButton(QWidget* _parent)
  :QPushButton(_parent)
{
  CTK_INIT_PRIVATE(ctkMenuButton);
}

//-----------------------------------------------------------------------------
ctkMenuButton::ctkMenuButton(const QString& title, QWidget* _parent)
  :QPushButton(title, _parent)
{
  CTK_INIT_PRIVATE(ctkMenuButton);
}

//-----------------------------------------------------------------------------
ctkMenuButton::~ctkMenuButton()
{
}

//-----------------------------------------------------------------------------
QSize ctkMenuButton::minimumSizeHint()const
{
  QSize min = QPushButton::minimumSizeHint();
  return QSize(min.width() + 13, min.height());
}

//-----------------------------------------------------------------------------
QSize ctkMenuButton::sizeHint()const
{
  QSize buttonSizeHint = QPushButton::sizeHint();
  return QSize(buttonSizeHint.width() + 13, buttonSizeHint.height());
}

//-----------------------------------------------------------------------------
void ctkMenuButton::paintEvent(QPaintEvent * _event)
{
  Q_UNUSED(_event);
  CTK_D(ctkMenuButton);
  QStylePainter painter(this);
  QStyleOptionButton option;
  initStyleOption(&option);
  bool drawIndicatorBackground = 
    option.state & QStyle::State_Sunken || 
    option.state & QStyle::State_On;
  // Draw button
  option.features &= ~QStyleOptionButton::HasMenu;
  if (this->menu() && (this->menu()->isVisible() || d->ShowMenu))
    {
    option.state &= ~QStyle::State_Sunken;
    option.state |= QStyle::State_Raised;
    }
  painter.drawControl(QStyle::CE_PushButtonBevel, option);
  // is PE_PanelButtonCommand better ?
  //painter.drawPrimitive(QStyle::PE_PanelButtonCommand, option);
  QRect downArrowRect = d->indicatorRect();
  if (drawIndicatorBackground)
    {    
    // if the button is down, draw the part under the indicator up
    QPixmap cache = QPixmap(option.rect.size());
    cache.fill(Qt::transparent);
    QPainter cachePainter(&cache);
    option.state &= ~QStyle::State_Sunken;
    option.state |= QStyle::State_Raised;
    option.state &= ~QStyle::State_On;
    option.state |= QStyle::State_Off;
    //option.state &= ~QStyle::State_HasFocus;
    option.state &= ~QStyle::State_MouseOver;
    this->style()->drawControl(QStyle::CE_PushButtonBevel, &option, &cachePainter, this);
    painter.drawItemPixmap(downArrowRect, Qt::AlignLeft | Qt::AlignTop, cache.copy(downArrowRect)); 
    }

  // Separator
  // Freely inspired by the painting of CC_ComboBox in qcleanlooksstyle.cpp 
  
  QColor buttonColor = this->palette().button().color();
  QColor darkColor;
  darkColor.setHsv(buttonColor.hue(),
                   qMin(255, (int)(buttonColor.saturation()*1.9)),
                   qMin(255, (int)(buttonColor.value()*0.7)));
  painter.setPen(buttonColor.darker(130));
  int borderSize = 2;
  painter.drawLine(QPoint(downArrowRect.left() - 1, downArrowRect.top() + borderSize),
                   QPoint(downArrowRect.left() - 1, downArrowRect.bottom() - borderSize));
  painter.setPen(this->palette().light().color());
  painter.drawLine(QPoint(downArrowRect.left(), downArrowRect.top() + borderSize),
                   QPoint(downArrowRect.left(), downArrowRect.bottom() - borderSize));
  // Draw arrow
  QStyleOption indicatorOpt;
  indicatorOpt.init(this);
  indicatorOpt.rect = downArrowRect.adjusted(borderSize, borderSize, -borderSize, -borderSize);
  painter.drawPrimitive(QStyle::PE_IndicatorArrowDown, indicatorOpt);

  // Draw Icon & Text
  option.rect.setRight( downArrowRect.left());
  painter.drawControl(QStyle::CE_PushButtonLabel, option);
}

//-----------------------------------------------------------------------------
bool ctkMenuButton::hitButton(const QPoint & _pos)const
{
  CTK_D(const ctkMenuButton);
  return !d->indicatorRect().contains(_pos) 
    && this->QPushButton::hitButton(_pos);
}

//-----------------------------------------------------------------------------
void ctkMenuButton::initStyleOption(QStyleOptionButton* option)const
{
  this->QPushButton::initStyleOption(option);
}

//-----------------------------------------------------------------------------
void ctkMenuButton::mousePressEvent(QMouseEvent *e)
{
  CTK_D(ctkMenuButton);
  // we don't want to open the menu if the mouse is clicked anywhere on
  // the button, only if it's clicked on the indecator
  this->disconnect(this,SIGNAL(pressed()), this, SLOT(_q_popupPressed()));
  this->QPushButton::mousePressEvent(e);
  if (e->isAccepted())
    {
    return;
    }
  if (d->indicatorRect().contains(e->pos()))
    {
    d->ShowMenu = true;
    this->showMenu();
    d->ShowMenu = false;
    e->accept();
    }
}
