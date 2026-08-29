package ui.views

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.DashPathEffect
import android.graphics.Paint
import android.graphics.RectF
import android.util.AttributeSet
import android.view.View

class VrCalibrationPreviewView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : View(context, attrs, defStyleAttr) {

    var playerHeightCm: Float = 175f
        set(value) {
            field = value
            invalidate()
        }

    var eyeOffsetMm: Float = 64f
        set(value) {
            field = value
            invalidate()
        }

    var eyeHeightOffsetCm: Float = 0f
        set(value) {
            field = value
            invalidate()
        }

    var isSeatedMode: Boolean = false
        set(value) {
            field = value
            invalidate()
        }

    private val gridPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.parseColor("#263D2A2E")
        strokeWidth = 1.5f
        style = Paint.Style.STROKE
    }

    private val groundPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.parseColor("#D4AF37")
        strokeWidth = 3f
        style = Paint.Style.STROKE
    }

    private val bodyPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.parseColor("#4D7A1D20")
        style = Paint.Style.FILL
    }

    private val bodyStrokePaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.parseColor("#7A1D20")
        strokeWidth = 2f
        style = Paint.Style.STROKE
    }

    private val headsetPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.parseColor("#E5C158")
        style = Paint.Style.FILL
    }

    private val eyePaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.parseColor("#00E5FF")
        style = Paint.Style.FILL
    }

    private val eyeRayPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.parseColor("#6600E5FF")
        strokeWidth = 2f
        style = Paint.Style.STROKE
        pathEffect = DashPathEffect(floatArrayOf(6f, 6f), 0f)
    }

    private val measurePaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.parseColor("#B3A2A4")
        strokeWidth = 2f
        style = Paint.Style.STROKE
    }

    private val textPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.parseColor("#F5EBE6")
        textSize = 28f
        isFakeBoldText = true
        textAlign = Paint.Align.LEFT
    }

    private val accentTextPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.parseColor("#E5C158")
        textSize = 24f
        isFakeBoldText = true
        textAlign = Paint.Align.CENTER
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        val w = width.toFloat()
        val h = height.toFloat()
        if (w <= 0 || h <= 0) return

        // Background subtle grid lines
        val gridStep = 40f
        var gx = 0f
        while (gx < w) {
            canvas.drawLine(gx, 0f, gx, h, gridPaint)
            gx += gridStep
        }
        var gy = 0f
        while (gy < h) {
            canvas.drawLine(0f, gy, w, gy, gridPaint)
            gy += gridStep
        }

        // Ground level line
        val groundY = h - 36f
        canvas.drawLine(20f, groundY, w - 20f, groundY, groundPaint)

        // Center X for avatar
        val avatarCenterX = w * 0.42f

        // Scaling reference: 220cm corresponds to max avatar height
        val maxDisplayHeight = groundY - 50f
        val heightRatio = (playerHeightCm.coerceIn(80f, 230f) / 230f)
        val currentHeadY = groundY - (maxDisplayHeight * heightRatio) - (eyeHeightOffsetCm * 1.5f)

        // Draw Avatar Body
        if (isSeatedMode) {
            // Seated body
            val seatTop = groundY - (maxDisplayHeight * heightRatio * 0.5f)
            // Torso
            val torsoRect = RectF(avatarCenterX - 28f, currentHeadY + 36f, avatarCenterX + 28f, seatTop)
            canvas.drawRoundRect(torsoRect, 12f, 12f, bodyPaint)
            canvas.drawRoundRect(torsoRect, 12f, 12f, bodyStrokePaint)

            // Thighs (horizontal)
            val thighRect = RectF(avatarCenterX - 28f, seatTop - 12f, avatarCenterX + 45f, seatTop + 14f)
            canvas.drawRoundRect(thighRect, 8f, 8f, bodyPaint)
            canvas.drawRoundRect(thighRect, 8f, 8f, bodyStrokePaint)

            // Lower legs (down to ground)
            val legRect = RectF(avatarCenterX + 25f, seatTop + 10f, avatarCenterX + 45f, groundY)
            canvas.drawRoundRect(legRect, 8f, 8f, bodyPaint)
            canvas.drawRoundRect(legRect, 8f, 8f, bodyStrokePaint)
        } else {
            // Standing body
            val hipY = currentHeadY + (groundY - currentHeadY) * 0.48f
            // Torso
            val torsoRect = RectF(avatarCenterX - 30f, currentHeadY + 34f, avatarCenterX + 30f, hipY)
            canvas.drawRoundRect(torsoRect, 14f, 14f, bodyPaint)
            canvas.drawRoundRect(torsoRect, 14f, 14f, bodyStrokePaint)

            // Left leg
            val leftLegRect = RectF(avatarCenterX - 26f, hipY - 6f, avatarCenterX - 4f, groundY)
            canvas.drawRoundRect(leftLegRect, 8f, 8f, bodyPaint)
            canvas.drawRoundRect(leftLegRect, 8f, 8f, bodyStrokePaint)

            // Right leg
            val rightLegRect = RectF(avatarCenterX + 4f, hipY - 6f, avatarCenterX + 26f, groundY)
            canvas.drawRoundRect(rightLegRect, 8f, 8f, bodyPaint)
            canvas.drawRoundRect(rightLegRect, 8f, 8f, bodyStrokePaint)
        }

        // Head Base
        val headRadius = 22f
        val headCenterY = currentHeadY + headRadius
        canvas.drawCircle(avatarCenterX, headCenterY, headRadius, bodyPaint)
        canvas.drawCircle(avatarCenterX, headCenterY, headRadius, bodyStrokePaint)

        // VR Headset Frame
        val headsetRect = RectF(avatarCenterX - 24f, headCenterY - 14f, avatarCenterX + 24f, headCenterY + 14f)
        canvas.drawRoundRect(headsetRect, 8f, 8f, headsetPaint)

        // Headset Strap
        val strapPaint = Paint(bodyStrokePaint).apply { strokeWidth = 3f }
        canvas.drawLine(avatarCenterX - 22f, headCenterY, avatarCenterX - 32f, headCenterY + 4f, strapPaint)
        canvas.drawLine(avatarCenterX + 22f, headCenterY, avatarCenterX + 32f, headCenterY + 4f, strapPaint)

        // Dual Eyes with dynamic IPD separation
        val ipdSpacingScale = (eyeOffsetMm - 55f) / (75f - 55f) // 0.0 to 1.0
        val eyeSeparation = 8f + (ipdSpacingScale * 14f) // visual spread

        val leftEyeX = avatarCenterX - eyeSeparation
        val rightEyeX = avatarCenterX + eyeSeparation
        val eyeY = headCenterY

        canvas.drawCircle(leftEyeX, eyeY, 4.5f, eyePaint)
        canvas.drawCircle(rightEyeX, eyeY, 4.5f, eyePaint)

        // Eye Forward Projection Rays
        canvas.drawLine(leftEyeX, eyeY, leftEyeX, eyeY - 40f, eyeRayPaint)
        canvas.drawLine(rightEyeX, eyeY, rightEyeX, eyeY - 40f, eyeRayPaint)
        canvas.drawLine(leftEyeX, eyeY - 40f, rightEyeX, eyeY - 40f, eyeRayPaint)

        // IPD Measurement Marker (Top of headset)
        canvas.drawText("${eyeOffsetMm.toInt()} mm IPD", avatarCenterX, eyeY - 48f, accentTextPaint)

        // Height Measurement Guide (Right side)
        val guideX = w * 0.82f
        canvas.drawLine(avatarCenterX + 38f, eyeY, guideX, eyeY, measurePaint)
        canvas.drawLine(guideX, eyeY, guideX, groundY, measurePaint)
        canvas.drawLine(guideX - 8f, eyeY, guideX + 8f, eyeY, measurePaint)
        canvas.drawLine(guideX - 8f, groundY, guideX + 8f, groundY, measurePaint)

        // Feet & Inches conversion
        val totalInches = (playerHeightCm / 2.54f).toInt()
        val feet = totalInches / 12
        val inches = totalInches % 12

        val heightLabel = "${playerHeightCm.toInt()} cm"
        val imperialLabel = "${feet}'${inches}\""

        canvas.drawText(heightLabel, guideX - 100f, (eyeY + groundY) / 2f - 6f, textPaint)
        canvas.drawText(imperialLabel, guideX - 100f, (eyeY + groundY) / 2f + 24f, accentTextPaint)

        // Stance mode label at bottom
        val stanceText = if (isSeatedMode) "SEATED MODE" else "STANDING MODE"
        val stancePaint = Paint(accentTextPaint).apply {
            textAlign = Paint.Align.LEFT
            textSize = 20f
        }
        canvas.drawText(stanceText, 24f, groundY - 12f, stancePaint)
    }
}
