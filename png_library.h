#pragma once

#include <windows.h>
#include <graphics.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// PNG图片类型（完全兼容EasyX的IMAGE）
typedef struct {
    void* gdiBitmap;    // GDI+ Bitmap对象（内部使用）
    int   width;        // 图片宽度
    int   height;       // 图片高度
} IMAGE_PNG;

// ============================================================
// 初始化PNG库（程序开始时调用一次）
// ============================================================
void init_png_library();

// ============================================================
// 清理PNG库（程序结束时调用一次）
// ============================================================
void cleanup_png_library();

// ============================================================
// 加载PNG图片（支持透明通道）
// 参数：
//   pImg     - 图片变量地址（IMAGE_PNG*）
//   filename - 图片文件路径（支持宽字符）
// ============================================================
void LoadPNG(IMAGE_PNG* pImg, const wchar_t* filename);

// ============================================================
// 绘制PNG图片（支持透明、缩放）
// 参数：
//   x, y    - 绘制位置
//   width   - 绘制宽度（0=使用原宽度）
//   height  - 绘制高度（0=使用原高度）
//   pImg    - 图片变量地址
//   alpha   - 透明度（0-255，255=不透明）
// ============================================================
void PutPNGEX(int x, int y, int width, int height,
    IMAGE_PNG* pImg, int alpha);

// ============================================================
// 释放PNG图片资源
// ============================================================
void FreePNG(IMAGE_PNG* pImg);

// ============================================================
// 旋转PNG图片（保持原图大小，像素不会超出）
// 参数：
//   pDestImg - 旋转后的图片（输出）
//   pSrcImg  - 源图片
//   angle    - 旋转角度（度，正数顺时针）
//   bkcolor  - 背景颜色（ARGB格式）
//   highqual - 是否高质量（1=是，0=否）
// ============================================================
void RotatePNG(IMAGE_PNG* pDestImg, IMAGE_PNG* pSrcImg,
    double angle, DWORD bkcolor, int highqual);

// ============================================================
// 绘制旋转后的PNG（一步到位，保持大小不变）
// 参数：
//   x, y    - 绘制位置
//   angle   - 旋转角度（度）
//   pImg    - 源图片
//   alpha   - 透明度（0-255）
//   bkcolor - 背景颜色（透明色）
// ============================================================
void PutPNGRotate(int x, int y, double angle, IMAGE_PNG* pImg,
    int alpha, DWORD bkcolor);

// ============================================================
// 旋转并缩放绘制（保持旋转后大小不变）
// 参数：
//   x, y    - 绘制位置
//   width   - 绘制宽度
//   height  - 绘制高度
//   angle   - 旋转角度（度）
//   pImg    - 源图片
//   alpha   - 透明度（0-255）
//   bkcolor - 背景颜色（透明色）
// ============================================================
void PutPNGRotateEx(int x, int y, int width, int height,
    double angle, IMAGE_PNG* pImg,
    int alpha, DWORD bkcolor);

// ============================================================
// 优化版：直接旋转绘制（不创建临时图片，性能更好）
// ============================================================
void PutPNGRotateDirect(int x, int y, double angle, IMAGE_PNG* pImg,
    int alpha, DWORD bkcolor);