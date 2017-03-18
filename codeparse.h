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
  uint8_t LcdType;
} ConfigTypeDef;

/**
 * @brief  操作数据包ID号，同一个数据包只能有一个ACTION.
 */
typedef enum
{
    ACT_RE_INIT_START,     /*< 重新初始化开始标志*/
    ACT_LCD_READ,          /*< 回读LCD寄存器，高速*/
    ACT_LCD_WRITE,         /*< 写LCD寄存器，高速模式*/
    ACT_SET_FRAME,         /*< 选择显示的画面*/
    ACT_FLASH_PARA,        /*< 固化调试好的参数到Flash*/
    ACT_FLASH_CONFIG_FILE, /*< 烧录配置文件 */
    ACT_CHANNEL_SEL,       /*< 选择通道 */
    ACT_UPGRADE_FIRMWARE,  /*< 更新固件 */
    ACT_REBOOT,
    ACT_GET_VERSION,
    ACT_READ_SSD2828,
    ACT_SET_SSD2828,
    ACT_RESET_SSD2828,
    ACT_SET_KEY,
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

typedef enum
{
    MIPI_LCD,
    RGB_SPI16BIT,
    RGB_SPI8BIT,
    RGB_SPI9BIT,
    SPI_2_Data_Lane,
}LcdTypeDef;

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
      RGB_START,
      RGB_SPI_RISING,
      RGB_SPI_FALLING,
      RGB_DELAY,
      RGB_WRITE,
      RGB_READ,
      RGB_END
  }RGBTypeDef;

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
      SHOW_ID,
      SLEEP_IN,
      SLEEP_OUT,
      PATTERN_END
  } PatternTypeDef;

  /**
   * @breif  LCD时序参数定义
   */
  typedef struct
  {
    quint16 DCLK;
    quint16 LCDH;
    quint16 LCDV;

    quint16 HBPD;
    quint16 HFPD;
    quint16 HSPW;

    quint16 VBPD;
    quint16 VFPD;
    quint16 VSPW;
  } LCDTimingParaTypeDef;

  LCDTimingParaTypeDef mLCDTiming;
  bool IsLcdTimingParsed;
public:


  CodeParse(QObject *parent);

  bool parseProjectName(QString data);
  bool parsePower(QString data);
  bool parseBacklight(QString data);
  bool parseLcdPara(QString data);
  bool parseMipiOr8BitRGBLcdInit(QString data);
  bool parseMipiSettings(QString data);
  bool parsePattern(QString data);
  bool parseAutoRun(QString data);
  bool parseLcdType(QString data);
  bool parseRGBLcdInit(QString data);

  bool compile(void);
  void updateStr(QString& str);

  QByteArray DataToSerial;

public slots:

signals:
  void Info(QString str);
};

#endif // CODEPARSE_H
