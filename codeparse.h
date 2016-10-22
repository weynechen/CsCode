#ifndef CODEPARSE_H
#define CODEPARSE_H
#include "QString.h"
#include "QObject"

#define  uint8_t           quint8

/**
 * @brief  系统配置数据长度
 */
#define LCD_PARA_LEN       18
#define LCD_INIT_LEN       3000
#define PATTERN_LEN        1024
#define MIPI_CONFIG_LEN    256
#define MAX_NAME_LEN       128

/**
 * @brief  系统配置数据结构体，这一部分数据会被烧录到flash
 */
typedef struct
{
  uint8_t PowerSettings;                 /*< 电源设置 */
  uint8_t Backlight;                     /*< 背光电流设置 */
  uint8_t LCDTimingPara[LCD_PARA_LEN];   /*< LCD 时序参数设置 */
  uint8_t LCDInitCode[LCD_INIT_LEN];     /*< LCD 初始化设置 */
  uint8_t MIPIConfig[MIPI_CONFIG_LEN];   /*< MIPI 参数设置 */
  uint8_t Pattern[PATTERN_LEN];          /*< pattern 设置 */
  uint8_t ProjectName[MAX_NAME_LEN];     /*< 项目名称设置 */
  uint8_t IsAutoRun;                     /*< 是否自动跑 */
} ConfigTypeDef;

/**
 * @brief  操作数据包ID号，同一个数据包只能有一个ACTION.
 */
typedef enum
{
  RE_INIT_START,       /*< 重新初始化开始标志*/
  LCD_READ,            /*< 回读LCD寄存器，高速*/
  LCD_WRITE,           /*< 写LCD寄存器，高速模式*/
  SET_FRAME,           /*< 选择显示的画面*/
  FLASH_PARA,          /*< 固化调试好的参数到Flash*/
  FLASH_CONFIG_FILE,   /*< 烧录配置文件 */
  CHANNEL_SEL,         /*< 选择通道 */
  UPDATE_FIRMWARE,     /*< 更新固件 */
  ACTION_NULL = 0xff   /*< 空动作*/
} ActionIDTypeDef;

/**
 * @brief  接口定义
 */
typedef enum
{
  IF_UART1,
  IF_USB,
  IF_UART3,
} InterfaceTypeDef;


class CodeParse : public QObject
{
  Q_OBJECT

private:

  const uint mMaxCurrent;
  QString mStrToParse;
  QStringList mTitleStr;
  QStringList mPowerStr;
  QStringList mLcdParaStr;
  QStringList mLcdInit;
  QString mProjectName;
  uint mPower;
  uint mBacklight;
  quint8 mAutoRun;
  QList<quint8> mCompiledPara;
  ConfigTypeDef mSystemConfig;

  typedef enum
  {
    NO_PACKAGE,
    GP,
    DCS
  } mipi_package;

  typedef enum
  {
    MIPI_START,
    MIPI_DCS,
    MIPI_GP,
    MIPI_DELAY,
    MIPI_WRITE,
    MIPI_READ,
    MIPI_END
  } mipi_type;
  typedef enum
  {
    PATTERN_START,
    RGB,
    FLICKERV,
    FLICKERH,
    COLORBARV,
    COLORBARH,
    GRAYLEVEL_V,
    GRAYLEVEL_H,
    CROSSTALK,
    PIC,
    FRAME,
    CHESSBOARD,
    RGBBAR,
    RGBLEVEL,
    PATTERN_STAY,
    NULL_PATTERN,
    PATTERN_END
  } pattern_type;


public:


  CodeParse(QObject *parent);

  bool parseProjectName(QString data);
  bool parsePower(QString data);
  bool parseBacklight(QString data);
  bool parseLcdPara(QString data);
  bool parseLcdInit(QString data);
  bool parseMipi(QString data);
  bool parsePattern(QString data);
  bool parseAutoRun(QString data);
  bool compile(void);
  void updateStr(QString& str);

  QByteArray DataToSerial;

public slots:

signals:
  void Info(QString str);
};

#endif // CODEPARSE_H
