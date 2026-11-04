/**
 * @file Adafruit_ST7735_API.cpp
 * @author Kyle Schmottlach (kjschmottlach@wpi.edu)
 * @brief This is the source code implementing the API defined for the Adafruit ST7735 TFT display driver chip. This API is declared in Adafruit_ST7735_API.h 
 *        and is intended to be used as a C-style wrapper around the C++ Adafruit_ST7735 class, allowing for easier integration with C code. The functions in 
 *        this file simply call the corresponding methods of the Adafruit_ST7735 class instance, which is stored in a global handle struct. This class is
 *        intended to be used with the 1.8" TFT shield variant used within the ECE 2049 course, and is entirely self-contained
 * @version 1.0
 * @date 2026-03-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "Adafruit_ST7735_API.h"
#include "Adafruit_ST7735.h"

struct ST7735_Handle {
    Adafruit_ST7735 instance;
};

ST7735_Handle *st7735_handle;

#ifdef __cplusplus
extern "C" {
#endif

volatile bool spiTxDone = false;

#ifdef __cplusplus
}
#endif //__cplusplus

//--------------------------------------------------------------------------------------------
// Adafruit_ST7735.cpp function definitions

void Display_create(SPI_HandleTypeDef *spi_handle, int16_t cs, GPIO_TypeDef *cs_port, int16_t dc, GPIO_TypeDef *dc_port) {
    static ST7735_Handle st7735 = ST7735_Handle{Adafruit_ST7735(spi_handle, cs, cs_port, dc, dc_port, -1, &spiTxDone)};
    st7735_handle = &st7735;
}

void Display_init(uint8_t options) {
    st7735_handle->instance.initR(options);
}

void Display_setRotation(uint8_t m) {
    st7735_handle->instance.setRotation(m);
}

//--------------------------------------------------------------------------------------------
// Adafruit_ST77xx.cpp function definitions

void Display_setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    st7735_handle->instance.setAddrWindow(x, y, w, h);
}

void Display_enableDisplay(bool enable) {
    st7735_handle->instance.enableDisplay(enable);
}

void Display_enableTearing(bool enable) {
    st7735_handle->instance.enableTearing(enable);
}

void Display_enableSleep(bool enable) {
    st7735_handle->instance.enableSleep(enable);
}

//--------------------------------------------------------------------------------------------
// Adafruit_SPITFT.cpp function definitions

void Display_drawPixel(int16_t x, int16_t y, uint16_t color) {
    st7735_handle->instance.drawPixel(x, y, color);
}

void Display_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    st7735_handle->instance.fillRect(x, y, w, h, color);
}

void Display_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    st7735_handle->instance.drawFastHLine(x, y, w, color);
}

void Display_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    st7735_handle->instance.drawFastVLine(x, y, h, color);
}

void Display_invertDisplay(bool i) {
    st7735_handle->instance.invertDisplay(i);
}

uint16_t Display_color565(uint8_t r, uint8_t g, uint8_t b) {
    return st7735_handle->instance.color565(r, g, b);
}

void Display_drawRGBBitmap(int16_t x, int16_t y, uint16_t *pcolors, int16_t w, int16_t h) {
    st7735_handle->instance.drawRGBBitmap(x, y, pcolors, w, h);
}
void Display_drawRGBBitmapMask(int16_t x, int16_t y, uint16_t *pcolors, int16_t w, int16_t h, uint8_t *mask) {
    st7735_handle->instance.drawRGBBitmap(x, y, pcolors, mask, w, h);
}

//--------------------------------------------------------------------------------------------
// Adafruit_GFX.cpp function definitions
void Display_fillScreen(uint16_t color) {
    st7735_handle->instance.fillScreen(color);
}
void Display_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color){
    st7735_handle->instance.drawLine(x0, y0, x1, y1, color);
}
void Display_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
    st7735_handle->instance.drawRect(x, y, w, h, color);
}
void Display_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color){
    st7735_handle->instance.drawCircle(x0, y0, r, color);
}
void Display_drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color){
    st7735_handle->instance.drawCircleHelper(x0, y0, r, cornername, color);
}
void Display_fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color){
    st7735_handle->instance.fillCircle(x0, y0, r, color);
}
void Display_fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color){
    st7735_handle->instance.fillCircleHelper(x0, y0, r, cornername, delta, color);
}
void Display_drawEllipse(int16_t x0, int16_t y0, int16_t rw, int16_t rh, uint16_t color){
    st7735_handle->instance.drawEllipse(x0, y0, rw, rh, color);
}
void Display_fillEllipse(int16_t x0, int16_t y0, int16_t rw, int16_t rh, uint16_t color){
    st7735_handle->instance.fillEllipse(x0, y0, rw, rh, color);
}
void Display_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color){
    st7735_handle->instance.drawTriangle(x0, y0, x1, y1, x2, y2, color);
}
void Display_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color){
    st7735_handle->instance.fillTriangle(x0, y0, x1, y1, x2, y2, color);
}
void Display_drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color){
    st7735_handle->instance.drawRoundRect(x0, y0, w, h, radius, color);
}
void Display_fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color){
    st7735_handle->instance.fillRoundRect(x0, y0, w, h, radius, color);
}
void Display_drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color){
    st7735_handle->instance.drawBitmap(x, y, bitmap, w, h, color);
}
void Display_drawBitmapBg(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg){
    st7735_handle->instance.drawBitmap(x, y, bitmap, w, h, color, bg);
}
void Display_drawXBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color){
    st7735_handle->instance.drawXBitmap(x, y, bitmap, w, h, color);
}
void Display_drawXBitmapBg(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg){
    st7735_handle->instance.drawXBitmap(x, y, bitmap, w, h, color, bg);
}
void Display_drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h){
    st7735_handle->instance.drawGrayscaleBitmap(x, y, bitmap, w, h);
}
void Display_drawGrayscaleBitmapMask(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint8_t *mask){
    st7735_handle->instance.drawGrayscaleBitmap(x, y, bitmap, mask, w, h);
}
void Display_drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y){
    st7735_handle->instance.drawChar(x, y, c, color, bg, size_x, size_y);
}
void Display_getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h){
    st7735_handle->instance.getTextBounds(string, x, y, x1, y1, w, h);
}
void Display_setTextSize(uint8_t sx, uint8_t sy){
    st7735_handle->instance.setTextSize(sx, sy);
}
void Display_setFont(const GFXfont *f){
    st7735_handle->instance.setFont(f);
}
void Display_setCursor(int16_t x, int16_t y){
    st7735_handle->instance.setCursor(x, y);
}
void Display_setTextColor(uint16_t c){
    st7735_handle->instance.setTextColor(c);
}
void Display_setTextColorWithBg(uint16_t c, uint16_t bg){
    st7735_handle->instance.setTextColor(c, bg);
}
void Display_setTextWrap(bool w){
    st7735_handle->instance.setTextWrap(w);
}
void Display_cp437(bool x){
    st7735_handle->instance.cp437(x);
}
int16_t Display_width(){
    return st7735_handle->instance.width();
}
int16_t Display_height(){
    return st7735_handle->instance.height();
}
uint8_t Display_getRotation(){
    return st7735_handle->instance.getRotation();
}
int16_t Display_getCursorX(){
    return st7735_handle->instance.getCursorX();
}
int16_t Display_getCursorY(){
    return st7735_handle->instance.getCursorY();
}

void Display_print(const char *str) {
    st7735_handle->instance.print(str);
}

void Display_println(const char *str) {
    st7735_handle->instance.println(str);
}