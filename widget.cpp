#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    delayLevel[0] = {1000};
    delayLevel[1] = {500};
    delayLevel[2] = {200};
    delayLevel[3] = {50};
    delayLevel[4] = {10};

    grade = 0;
    winFlag = 0;
    failFlag = 0;
    snakeBody[0] = QPoint(5,5);
    snakeBody[1] = snakeBody[0] + QPoint(-1,0);
    snakeBody[2] = snakeBody[1] + QPoint(-1,0);
    snakeLen = 3;
    snakeDir = QPoint(0,0);
    delayLevelIdx = DEFAULT_DELAY_LEVEL;

    runTimer.setInterval(delayLevel[delayLevelIdx]);
    runTimer.start();
    connect(&runTimer,&QTimer::timeout,[=](){
        SnakeRun();
    });
    updateTimer.setInterval(10);
    updateTimer.start();
    connect(&updateTimer,&QTimer::timeout,[=](){
        update();
    });
    CreateFood();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::GetDir()
{
    // 如果按下了方向键，切换蛇头方向，需要注意跨界时的反头
    if (keyDir != snakeBody[1] + QPoint(0,0) - snakeBody[0]
            && keyDir != snakeBody[1] + QPoint(ROW,0) - snakeBody[0]
            && keyDir != snakeBody[1] + QPoint(-ROW,0) - snakeBody[0]
            && keyDir != snakeBody[1] + QPoint(0,COL) - snakeBody[0]
            && keyDir != snakeBody[1] + QPoint(0,-COL) - snakeBody[0])
    {
        snakeDir = keyDir;
    }
    // 是否启用自动吃东西，仅用于debug
    if (0) {
        QPoint autoDir = food - snakeBody[0];
        int max = qMax(qAbs(autoDir.x()),qAbs(autoDir.y()));
        autoDir /= max;
        if (autoDir.x() != 0) {
            autoDir.ry() = 0;
        }
        snakeDir = autoDir;
    }
}

void Widget::SnakeRun()
{
    GetDir();
    // 蛇头有方向，那么蛇就运动
    if (QPoint(0,0) != snakeDir)
    {
        for (int i = snakeLen - 1;i > 0; i--)
        {
            snakeBody[i] = snakeBody[i-1];
        }
        snakeBody[0] += snakeDir;
        snakeBody[0].rx() = (snakeBody[0].x() + ROW) % ROW;
        snakeBody[0].ry() = (snakeBody[0].y() + COL) % COL;
        if(food == snakeBody[0])
        {
            // 将身子最后一节赋值，就不会显示到00了
            snakeBody[snakeLen] = snakeBody[snakeLen - 1];
            ++snakeLen;
            ++grade;
            CreateFood();
        }
    }
}
void Widget::CreateFood()
{
    int i,j;
    int emptyExist = 0;
    memset(snakeOccupy,0,sizeof(snakeOccupy));
    for ( i = 0;i < snakeLen; i++) {
        snakeOccupy[snakeBody[i].x()][snakeBody[i].y()] = 1;

    }
    for (i = 0;(0 == emptyExist) && i < COL; i++) {
        for (j = 0;(0 == emptyExist) &&  j < ROW; j ++) {
            if (0 == snakeOccupy[i][j])
            {
                // 存在空格，可以启用随机数放置食物
                emptyExist = 1;
            }
        }
    }
    // 如果已经没有空间放下食物，说明游戏胜利
    if (0 == emptyExist)
    {
        winFlag = 1;
    }
    while (emptyExist) {
        i = qrand() % ROW;
        j = qrand() % COL;
        if (0 == snakeOccupy[i][j]){
            food = QPoint(i,j);
            break;
        }

    }
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    if ("w" == event->text())
    {
        keyDir = QPoint(0,-1);
    }
    else if ("s" == event->text())
    {
        keyDir = QPoint(0,1);
    }
    else if ("a" == event->text())
    {
        keyDir = QPoint(-1,0);
    }
    else if ("d" == event->text())
    {
        keyDir = QPoint(1,0);
    }
    // 加入空格键，进行暂停操作
    else if ("z" == event->text())
    {
        keyDir = QPoint(0,0);
    }
    else if ("r" == event->text()) {
        grade = 0;
        winFlag = 0;
        failFlag = 0;
        snakeBody[0] = QPoint(5,5);
        snakeBody[1] = snakeBody[0] + QPoint(-1,0);
        snakeBody[2] = snakeBody[1] + QPoint(-1,0);
        snakeLen = 3;
        keyDir = QPoint(0,0);
        snakeDir = QPoint(0,0);
        delayLevelIdx = DEFAULT_DELAY_LEVEL;
        runTimer.setInterval(delayLevel[delayLevelIdx]);
    }
    else if ("q" == event->text()) {
        if (delayLevelIdx > 0) {
            -- delayLevelIdx;
            runTimer.setInterval(delayLevel[delayLevelIdx]);
        }
    }
    else if ("e" == event->text()) {
        if (delayLevelIdx < DELAY_LEVEL) {
            ++ delayLevelIdx;
            runTimer.setInterval(delayLevel[delayLevelIdx]);
        }
    }
}

void Widget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QString str = "food:" + QString::number(food.x()) + QString::number(food.y());
    painter.drawText(QPoint(COL,2) * SCALE,str);
    str = "grade="+QString::number(grade);
    painter.drawText(QPoint(COL,3) * SCALE,str);

    painter.drawLine(QPoint(COL,0) * SCALE, QPoint(COL,ROW) * SCALE);
    painter.drawLine(QPoint(0,ROW) * SCALE, QPoint(COL,ROW) * SCALE);

    for(int i = snakeLen-1; i >=0 ; i--)
    {
        painter.drawEllipse(snakeBody[i] * SCALE + QPoint(SCALE / 2,SCALE / 2),
                            SNAKE_WIDTH / 2, SNAKE_WIDTH / 2);
    }
    // 设置画刷，重画蛇头，突出食物位置
    painter.setBrush(QBrush(qRgb(0,255,0)));
    painter.drawEllipse(snakeBody[0] * SCALE + QPoint(SCALE / 2,SCALE / 2),
            SNAKE_WIDTH / 2, SNAKE_WIDTH / 2);
    painter.drawEllipse(food * SCALE + QPoint(SCALE / 2,SCALE / 2) ,
                        FOOD_WIDTH / 2,FOOD_WIDTH / 2);
    
    if (winFlag) {
        painter.drawText(QPoint(COL,4) * SCALE,"!!!win!!!");
    }
    else if (failFlag) {
        painter.drawText(QPoint(COL,4) * SCALE,"!!!fail!!!");
    }
    str = "speed["+QString::number(delayLevelIdx) + "]=" + QString::number(delayLevel[delayLevelIdx]);
    painter.drawText(QPoint(COL,5) * SCALE,str);
}
