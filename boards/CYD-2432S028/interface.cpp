#include "core/powerSave.h"
#include "core/utils.h"
#include <Arduino.h>
#include <interface.h>
#include <Wire.h>

/* * تعريفات أرجل Heltec WiFi LoRa 32 V3 
 */
#define HELTEC_VEXT   36   // التحكم في طاقة الشاشة والـ LoRa
#define HELTEC_BUTTON 0    // زر البرنامج (PRG) الموجود على اللوحة
#define OLED_SDA      17   // I2C Data
#define OLED_SCL      18   // I2C Clock
#define OLED_RST      21   // OLED Reset

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description: الإعدادات الأولية للأرجل في لوحة Heltec V3
***************************************************************************************/
void _setup_gpio() {
    // 1. تفعيل مخرج الطاقة (Vext) لتشغيل الشاشة والـ LoRa
    pinMode(HELTEC_VEXT, OUTPUT);
    digitalWrite(HELTEC_VEXT, LOW); // القيمة LOW في V3 تعني تشغيل الطاقة
    
    // 2. إعداد الزر الفعلي للوحة ليعمل كمدخل (بديل للمس)
    pinMode(HELTEC_BUTTON, INPUT_PULLUP);

    // 3. إعادة ضبط الشاشة (OLED Reset)
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);

    // 4. تهيئة بروتوكول I2C بالأرجل المخصصة للهيلتك
    Wire.begin(OLED_SDA, OLED_SCL);

    bruceConfig.colorInverted = 0;
    Serial.println("Heltec V3 GPIO Setup Complete.");
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description: إعدادات ما بعد التشغيل (تم إزالة تعريفات الـ Backlight لشاشات TFT)
***************************************************************************************/
void _post_setup_gpio() {
    // شاشات OLED SSD1306 لا تستخدم رجل إضاءة خلفية (TFT_BL) 
    // لذا تم تفريغ هذه الدالة لتجنب تعارض الأرجل مع معالج ESP32-S3
}

/*********************************************************************
** Function: _setBrightness
** location: settings.cpp
** التحكم في السطوع (لشاشات OLED يتم برمجياً عبر الـ Contrast)
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    // في شاشات OLED SSD1306 لا يوجد PWM للسطوع 
    // السطوع يتم التحكم به عبر أوامر I2C مباشرة
    // إذا كنت تستخدم مكتبة Adafruit يمكن إضافة: display.setContrast(brightval);
}

/*********************************************************************
** Function: InputHandler
** معالجة المدخلات: تحويل ضغطة الزر الفعلي إلى حركة في القوائم
**********************************************************************/
void InputHandler(void) {
    static long d_tmp = 0;
    
    // قراءة الزر الموجود على اللوحة (GPIO 0)
    if (digitalRead(HELTEC_BUTTON) == LOW) {
        if (millis() - d_tmp > 200) { // Debounce لمنع تكرار الضغطة
            
            if (!wakeUpScreen()) {
                // إذا كانت الشاشة مستيقظة، نرسل أمر "التالي" للتنقل في القوائم
                NextPress = true; 
                AnyKeyPress = true;
            }
            
            d_tmp = millis();
        }
    }
}

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** إيقاف التشغيل والدخول في النوم العميق
**********************************************************************/
void powerOff() {
    // إطفاء مخرج الطاقة لتوفير البطارية
    digitalWrite(HELTEC_VEXT, HIGH); 
    
    // ضبط زر PRG ليقوم بإيقاظ اللوحة من النوم
    esp_sleep_enable_ext0_wakeup((gpio_num_t)HELTEC_BUTTON, LOW);
    esp_deep_sleep_start();
}

/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
**********************************************************************/
void checkReboot() {}
