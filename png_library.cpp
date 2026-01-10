#include "png_library.h"

// 全局GDI+令牌
static ULONG_PTR g_gdiplusToken = 0;

// ============================================================
// 初始化PNG库（程序开始时调用一次）
// ============================================================
void init_png_library() {
    if (g_gdiplusToken == 0) {
        GdiplusStartupInput gdiplusStartupInput;
        GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
    }
}

// ============================================================
// 清理PNG库（程序结束时调用一次）
// ============================================================
void cleanup_png_library() {
    if (g_gdiplusToken != 0) {
        GdiplusShutdown(g_gdiplusToken);
        g_gdiplusToken = 0;
    }
}

// ============================================================
// 加载PNG图片（支持透明通道）
// ============================================================
void LoadPNG(IMAGE_PNG* pImg, const wchar_t* filename) {
    // 清空结构体
    memset(pImg, 0, sizeof(IMAGE_PNG));

    // 检查库是否初始化
    if (g_gdiplusToken == 0) {
        init_png_library();
    }

    // 使用GDI+加载PNG
    Bitmap* bitmap = Bitmap::FromFile(filename);
    if (bitmap && bitmap->GetLastStatus() == Ok) {
        pImg->gdiBitmap = bitmap;
        pImg->width = bitmap->GetWidth();
        pImg->height = bitmap->GetHeight();
    }
    else {
        delete bitmap;
    }
}

// ============================================================
// 绘制PNG图片（支持透明、缩放）
// ============================================================
void PutPNGEX(int x, int y, int width, int height,
    IMAGE_PNG* pImg, int alpha) {
    // 检查图片是否有效
    if (!pImg || !pImg->gdiBitmap) return;

    // 获取EasyX窗口的设备上下文
    HDC hdc = GetImageHDC(NULL);

    // 创建GDI+绘图对象
    Graphics graphics(hdc);
    Bitmap* bitmap = (Bitmap*)pImg->gdiBitmap;

    // 确定绘制尺寸
    int drawWidth = (width <= 0) ? pImg->width : width;
    int drawHeight = (height <= 0) ? pImg->height : height;

    // 设置图像属性（透明度）
    ImageAttributes attr;
    if (alpha < 255) {
        float a = alpha / 255.0f;
        ColorMatrix matrix = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,   a, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        attr.SetColorMatrix(&matrix);
    }

    // 绘制图片
    Rect destRect(x, y, drawWidth, drawHeight);
    graphics.DrawImage(bitmap, destRect,
        0, 0, pImg->width, pImg->height,
        UnitPixel,
        (alpha < 255) ? &attr : NULL);
}

// ============================================================
// 释放PNG图片资源
// ============================================================
void FreePNG(IMAGE_PNG* pImg) {
    if (pImg && pImg->gdiBitmap) {
        delete (Bitmap*)pImg->gdiBitmap;
        pImg->gdiBitmap = NULL;
        pImg->width = 0;
        pImg->height = 0;
    }
}

// ============================================================
// 旋转PNG图片（保持原图大小，像素不会超出）
// ============================================================
void RotatePNG(IMAGE_PNG* pDestImg, IMAGE_PNG* pSrcImg,
    double angle, DWORD bkcolor, int highqual) {
    // 检查源图片是否有效
    if (!pSrcImg || !pSrcImg->gdiBitmap) {
        memset(pDestImg, 0, sizeof(IMAGE_PNG));
        return;
    }

    Bitmap* srcBitmap = (Bitmap*)pSrcImg->gdiBitmap;
    int srcWidth = pSrcImg->width;
    int srcHeight = pSrcImg->height;

    // 创建目标位图（保持原图大小）
    Bitmap* destBitmap = new Bitmap(srcWidth, srcHeight,
        PixelFormat32bppARGB);

    // 设置图形对象
    Graphics graphics(destBitmap);

    // 设置插值模式（影响旋转质量）
    if (highqual) {
        graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    }
    else {
        graphics.SetInterpolationMode(InterpolationModeNearestNeighbor);
    }

    // 设置背景色
    Color backgroundColor(
        (bkcolor >> 24) & 0xFF,  // A
        (bkcolor >> 16) & 0xFF,  // R
        (bkcolor >> 8) & 0xFF,   // G
        bkcolor & 0xFF           // B
    );
    graphics.Clear(backgroundColor);

    // 计算旋转中心（图片中心）
    REAL centerX = (REAL)srcWidth / 2;
    REAL centerY = (REAL)srcHeight / 2;

    // 设置变换矩阵
    graphics.TranslateTransform(centerX, centerY);  // 移动到中心
    graphics.RotateTransform((REAL)angle);          // 旋转
    graphics.TranslateTransform(-centerX, -centerY); // 移回

    // 绘制旋转后的图片（保持原位置和大小）
    graphics.DrawImage(srcBitmap, 0, 0, srcWidth, srcHeight);

    // 更新目标图片信息（大小保持不变）
    if (pDestImg->gdiBitmap) {
        delete (Bitmap*)pDestImg->gdiBitmap;
    }

    pDestImg->gdiBitmap = destBitmap;
    pDestImg->width = srcWidth;      // 保持原宽度
    pDestImg->height = srcHeight;    // 保持原高度
}

// ============================================================
// 绘制旋转后的PNG（一步到位，保持大小不变）
// ============================================================
void PutPNGRotate(int x, int y, double angle, IMAGE_PNG* pImg,
    int alpha, DWORD bkcolor) {
    if (!pImg || !pImg->gdiBitmap) return;

    // 临时图片用于旋转
    IMAGE_PNG rotatedImg;
    memset(&rotatedImg, 0, sizeof(IMAGE_PNG));

    // 旋转图片（保持大小不变，高质量）
    RotatePNG(&rotatedImg, pImg, angle, bkcolor, 1);

    // 绘制旋转后的图片
    if (rotatedImg.gdiBitmap) {
        HDC hdc = GetImageHDC(NULL);
        Graphics graphics(hdc);
        Bitmap* bitmap = (Bitmap*)rotatedImg.gdiBitmap;

        // 设置图像属性（透明度）
        ImageAttributes attr;
        if (alpha < 255) {
            float a = alpha / 255.0f;
            ColorMatrix matrix = {
                1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f,   a, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f, 1.0f
            };
            attr.SetColorMatrix(&matrix);
        }

        // 绘制图片（大小不变）
        Rect destRect(x, y, rotatedImg.width, rotatedImg.height);
        graphics.DrawImage(bitmap, destRect,
            0, 0, rotatedImg.width, rotatedImg.height,
            UnitPixel,
            (alpha < 255) ? &attr : NULL);

        // 释放临时图片
        FreePNG(&rotatedImg);
    }
}

// ============================================================
// 旋转并缩放绘制（保持旋转后大小不变）
// ============================================================
void PutPNGRotateEx(int x, int y, int width, int height,
    double angle, IMAGE_PNG* pImg,
    int alpha, DWORD bkcolor) {
    if (!pImg || !pImg->gdiBitmap) return;

    // 临时图片用于旋转
    IMAGE_PNG rotatedImg;
    memset(&rotatedImg, 0, sizeof(IMAGE_PNG));

    // 旋转图片（保持原图大小）
    RotatePNG(&rotatedImg, pImg, angle, bkcolor, 1);

    // 绘制旋转后的图片（支持缩放）
    if (rotatedImg.gdiBitmap) {
        HDC hdc = GetImageHDC(NULL);
        Graphics graphics(hdc);
        Bitmap* bitmap = (Bitmap*)rotatedImg.gdiBitmap;

        // 确定绘制尺寸（如果width/height为0，使用原尺寸）
        int drawWidth = (width <= 0) ? rotatedImg.width : width;
        int drawHeight = (height <= 0) ? rotatedImg.height : height;

        // 设置图像属性（透明度）
        ImageAttributes attr;
        if (alpha < 255) {
            float a = alpha / 255.0f;
            ColorMatrix matrix = {
                1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f,   a, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f, 1.0f
            };
            attr.SetColorMatrix(&matrix);
        }

        // 绘制图片（可以缩放）
        Rect destRect(x, y, drawWidth, drawHeight);
        graphics.DrawImage(bitmap, destRect,
            0, 0, rotatedImg.width, rotatedImg.height,
            UnitPixel,
            (alpha < 255) ? &attr : NULL);

        // 释放临时图片
        FreePNG(&rotatedImg);
    }
}

// ============================================================
// 优化版：直接旋转绘制（不创建临时图片，性能更好）
// ============================================================
void PutPNGRotateDirect(int x, int y, double angle, IMAGE_PNG* pImg,
    int alpha, DWORD bkcolor) {
    if (!pImg || !pImg->gdiBitmap) return;

    HDC hdc = GetImageHDC(NULL);
    Graphics graphics(hdc);
    Bitmap* srcBitmap = (Bitmap*)pImg->gdiBitmap;

    // 保存原始绘图状态
    GraphicsState state = graphics.Save();

    // 移动到绘制位置
    graphics.TranslateTransform((REAL)x + pImg->width / 2,
        (REAL)y + pImg->height / 2);
    // 旋转
    graphics.RotateTransform((REAL)angle);
    // 移回以便图片居中
    graphics.TranslateTransform(-(REAL)pImg->width / 2,
        -(REAL)pImg->height / 2);

    // 设置图像属性（透明度）
    ImageAttributes attr;
    if (alpha < 255) {
        float a = alpha / 255.0f;
        ColorMatrix matrix = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,   a, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        attr.SetColorMatrix(&matrix);
    }

    // 设置背景色（如果需要透明背景，用这个技巧）
    if ((bkcolor & 0xFF000000) != 0xFF000000) {
        // 如果有透明度，使用CompositingMode
        graphics.SetCompositingMode(CompositingModeSourceOver);
    }

    // 绘制图片
    Rect destRect(0, 0, pImg->width, pImg->height);
    graphics.DrawImage(srcBitmap, destRect,
        0, 0, pImg->width, pImg->height,
        UnitPixel,
        (alpha < 255) ? &attr : NULL);

    // 恢复原始状态
    graphics.Restore(state);
}