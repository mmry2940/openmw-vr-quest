package ui.activity

import android.content.SharedPreferences
import android.os.Bundle
import android.preference.PreferenceManager
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.Toolbar
import com.google.android.material.button.MaterialButton
import com.google.android.material.slider.Slider
import com.libopenmw.openmw.R
import constants.Constants
import file.Writer
import ui.views.VrCalibrationPreviewView
import java.io.File
import java.util.Locale

class VrCalibrationActivity : AppCompatActivity() {

    private lateinit var prefs: SharedPreferences

    private lateinit var previewCanvas: VrCalibrationPreviewView
    private lateinit var stanceBadge: TextView
    private lateinit var summaryHeight: TextView
    private lateinit var summaryIpd: TextView
    private lateinit var summaryVOffset: TextView

    private lateinit var txtHeightValue: TextView
    private lateinit var btnModeStanding: MaterialButton
    private lateinit var btnModeSeated: MaterialButton
    private lateinit var sliderHeight: Slider

    private lateinit var txtIpdValue: TextView
    private lateinit var sliderIpd: Slider

    private lateinit var txtVOffsetValue: TextView
    private lateinit var sliderVOffset: Slider

    private lateinit var btnSaveCalibration: MaterialButton
    private lateinit var btnResetCalibration: MaterialButton

    private var currentHeightCm: Float = 175f
    private var currentIpdMm: Float = 64f
    private var currentVOffsetCm: Float = 0f
    private var isSeatedMode: Boolean = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_vr_calibration)

        prefs = PreferenceManager.getDefaultSharedPreferences(this)

        initViews()
        loadSavedSettings()
        setupListeners()
        updateUI()
    }

    private fun initViews() {
        val toolbar = findViewById<Toolbar>(R.id.toolbar)
        setSupportActionBar(toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        supportActionBar?.setDisplayShowHomeEnabled(true)
        toolbar.setNavigationOnClickListener {
            finish()
        }

        previewCanvas = findViewById(R.id.vr_preview_canvas)
        stanceBadge = findViewById(R.id.preview_badge_stance)
        summaryHeight = findViewById(R.id.txt_summary_height)
        summaryIpd = findViewById(R.id.txt_summary_ipd)
        summaryVOffset = findViewById(R.id.txt_summary_v_offset)

        txtHeightValue = findViewById(R.id.txt_height_value)
        btnModeStanding = findViewById(R.id.btn_mode_standing)
        btnModeSeated = findViewById(R.id.btn_mode_seated)
        sliderHeight = findViewById(R.id.slider_height)

        txtIpdValue = findViewById(R.id.txt_ipd_value)
        sliderIpd = findViewById(R.id.slider_ipd)

        txtVOffsetValue = findViewById(R.id.txt_v_offset_value)
        sliderVOffset = findViewById(R.id.slider_v_offset)

        btnSaveCalibration = findViewById(R.id.btn_save_calibration)
        btnResetCalibration = findViewById(R.id.btn_reset_calibration)
    }

    private fun loadSavedSettings() {
        currentHeightCm = try {
            prefs.getFloat("pref_vr_height_val", prefs.getString("pref_vr_height", "175.0")?.toFloatOrNull() ?: 175f)
        } catch (e: Exception) {
            175f
        }

        currentIpdMm = try {
            prefs.getFloat("pref_vr_eye_offset_val", prefs.getString("pref_vr_eye_offset", "64.0")?.toFloatOrNull() ?: 64f)
        } catch (e: Exception) {
            64f
        }

        currentVOffsetCm = try {
            prefs.getFloat("pref_vr_eye_height_offset_val", prefs.getString("pref_vr_eye_height_offset", "0.0")?.toFloatOrNull() ?: 0f)
        } catch (e: Exception) {
            0f
        }

        val stance = prefs.getString("pref_vr_stance", "standing")
        isSeatedMode = stance.equals("seated", ignoreCase = true)
    }

    private fun setupListeners() {
        // Mode toggles
        btnModeStanding.setOnClickListener {
            isSeatedMode = false
            if (currentHeightCm < 140f) {
                currentHeightCm = 175f
            }
            updateUI()
        }

        btnModeSeated.setOnClickListener {
            isSeatedMode = true
            if (currentHeightCm > 140f) {
                currentHeightCm = 120f
            }
            updateUI()
        }

        // Height slider
        sliderHeight.addOnChangeListener { _, value, fromUser ->
            if (fromUser) {
                currentHeightCm = value
                updateUI(fromHeightSlider = true)
            }
        }

        // Height fine-tune
        findViewById<Button>(R.id.btn_height_minus_5).setOnClickListener {
            currentHeightCm = (currentHeightCm - 5f).coerceIn(100f, 220f)
            updateUI()
        }
        findViewById<Button>(R.id.btn_height_minus_1).setOnClickListener {
            currentHeightCm = (currentHeightCm - 1f).coerceIn(100f, 220f)
            updateUI()
        }
        findViewById<Button>(R.id.btn_height_plus_1).setOnClickListener {
            currentHeightCm = (currentHeightCm + 1f).coerceIn(100f, 220f)
            updateUI()
        }
        findViewById<Button>(R.id.btn_height_plus_5).setOnClickListener {
            currentHeightCm = (currentHeightCm + 5f).coerceIn(100f, 220f)
            updateUI()
        }

        // Height chips
        findViewById<Button>(R.id.chip_height_120).setOnClickListener {
            isSeatedMode = true
            currentHeightCm = 120f
            updateUI()
        }
        findViewById<Button>(R.id.chip_height_165).setOnClickListener {
            isSeatedMode = false
            currentHeightCm = 165f
            updateUI()
        }
        findViewById<Button>(R.id.chip_height_175).setOnClickListener {
            isSeatedMode = false
            currentHeightCm = 175f
            updateUI()
        }
        findViewById<Button>(R.id.chip_height_185).setOnClickListener {
            isSeatedMode = false
            currentHeightCm = 185f
            updateUI()
        }
        findViewById<Button>(R.id.chip_height_195).setOnClickListener {
            isSeatedMode = false
            currentHeightCm = 195f
            updateUI()
        }

        // IPD slider
        sliderIpd.addOnChangeListener { _, value, fromUser ->
            if (fromUser) {
                currentIpdMm = value
                updateUI(fromIpdSlider = true)
            }
        }

        // IPD fine-tune
        findViewById<Button>(R.id.btn_ipd_minus_1).setOnClickListener {
            currentIpdMm = (currentIpdMm - 1f).coerceIn(55f, 75f)
            updateUI()
        }
        findViewById<Button>(R.id.btn_ipd_minus_05).setOnClickListener {
            currentIpdMm = (currentIpdMm - 0.5f).coerceIn(55f, 75f)
            updateUI()
        }
        findViewById<Button>(R.id.btn_ipd_plus_05).setOnClickListener {
            currentIpdMm = (currentIpdMm + 0.5f).coerceIn(55f, 75f)
            updateUI()
        }
        findViewById<Button>(R.id.btn_ipd_plus_1).setOnClickListener {
            currentIpdMm = (currentIpdMm + 1f).coerceIn(55f, 75f)
            updateUI()
        }

        // IPD chips
        findViewById<Button>(R.id.chip_ipd_58).setOnClickListener {
            currentIpdMm = 58f
            updateUI()
        }
        findViewById<Button>(R.id.chip_ipd_63).setOnClickListener {
            currentIpdMm = 63f
            updateUI()
        }
        findViewById<Button>(R.id.chip_ipd_65).setOnClickListener {
            currentIpdMm = 65f
            updateUI()
        }
        findViewById<Button>(R.id.chip_ipd_68).setOnClickListener {
            currentIpdMm = 68f
            updateUI()
        }
        findViewById<Button>(R.id.chip_ipd_72).setOnClickListener {
            currentIpdMm = 72f
            updateUI()
        }

        // Vertical Offset slider
        sliderVOffset.addOnChangeListener { _, value, fromUser ->
            if (fromUser) {
                currentVOffsetCm = value
                updateUI(fromVOffsetSlider = true)
            }
        }

        // Vertical Offset fine-tune
        findViewById<Button>(R.id.btn_v_offset_minus_1).setOnClickListener {
            currentVOffsetCm = (currentVOffsetCm - 1f).coerceIn(-10f, 10f)
            updateUI()
        }
        findViewById<Button>(R.id.btn_v_offset_zero).setOnClickListener {
            currentVOffsetCm = 0f
            updateUI()
        }
        findViewById<Button>(R.id.btn_v_offset_plus_1).setOnClickListener {
            currentVOffsetCm = (currentVOffsetCm + 1f).coerceIn(-10f, 10f)
            updateUI()
        }

        // Action buttons
        btnSaveCalibration.setOnClickListener {
            saveCalibration()
        }

        btnResetCalibration.setOnClickListener {
            currentHeightCm = 175f
            currentIpdMm = 64f
            currentVOffsetCm = 0f
            isSeatedMode = false
            updateUI()
            Toast.makeText(this, "Reset to recommended defaults", Toast.LENGTH_SHORT).show()
        }
    }

    private fun updateUI(
        fromHeightSlider: Boolean = false,
        fromIpdSlider: Boolean = false,
        fromVOffsetSlider: Boolean = false
    ) {
        // Stance mode styling
        if (isSeatedMode) {
            btnModeSeated.setBackgroundColor(resources.getColor(R.color.colorPrimary))
            btnModeSeated.strokeColor = android.content.res.ColorStateList.valueOf(resources.getColor(R.color.gold_accent))
            btnModeSeated.setTextColor(resources.getColor(R.color.text_primary))

            btnModeStanding.setBackgroundColor(resources.getColor(R.color.btn_action_surface))
            btnModeStanding.strokeColor = android.content.res.ColorStateList.valueOf(resources.getColor(R.color.surface_card_stroke))
            btnModeStanding.setTextColor(resources.getColor(R.color.text_secondary))
            stanceBadge.text = "Seated Mode"
        } else {
            btnModeStanding.setBackgroundColor(resources.getColor(R.color.colorPrimary))
            btnModeStanding.strokeColor = android.content.res.ColorStateList.valueOf(resources.getColor(R.color.gold_accent))
            btnModeStanding.setTextColor(resources.getColor(R.color.text_primary))

            btnModeSeated.setBackgroundColor(resources.getColor(R.color.btn_action_surface))
            btnModeSeated.strokeColor = android.content.res.ColorStateList.valueOf(resources.getColor(R.color.surface_card_stroke))
            btnModeSeated.setTextColor(resources.getColor(R.color.text_secondary))
            stanceBadge.text = "Standing Mode"
        }

        // Height label
        val totalInches = (currentHeightCm / 2.54f).toInt()
        val feet = totalInches / 12
        val inches = totalInches % 12
        val heightStr = "${currentHeightCm.toInt()} cm  •  ${feet} ft ${inches} in"
        txtHeightValue.text = heightStr
        summaryHeight.text = "${currentHeightCm.toInt()} cm (${feet}'${inches}\")"

        if (!fromHeightSlider) {
            sliderHeight.value = currentHeightCm.coerceIn(sliderHeight.valueFrom, sliderHeight.valueTo)
        }

        // IPD label
        val ipdStr = String.format(Locale.ROOT, "%.1f mm", currentIpdMm)
        txtIpdValue.text = ipdStr
        summaryIpd.text = ipdStr

        if (!fromIpdSlider) {
            sliderIpd.value = currentIpdMm.coerceIn(sliderIpd.valueFrom, sliderIpd.valueTo)
        }

        // Vertical Offset label
        val vOffsetStr = when {
            currentVOffsetCm > 0 -> String.format(Locale.ROOT, "+%.1f cm (Raised)", currentVOffsetCm)
            currentVOffsetCm < 0 -> String.format(Locale.ROOT, "%.1f cm (Lowered)", currentVOffsetCm)
            else -> "0.0 cm (Level)"
        }
        txtVOffsetValue.text = vOffsetStr
        summaryVOffset.text = String.format(Locale.ROOT, "%+.1f cm", currentVOffsetCm)

        if (!fromVOffsetSlider) {
            sliderVOffset.value = currentVOffsetCm.coerceIn(sliderVOffset.valueFrom, sliderVOffset.valueTo)
        }

        // Canvas update
        previewCanvas.playerHeightCm = currentHeightCm
        previewCanvas.eyeOffsetMm = currentIpdMm
        previewCanvas.eyeHeightOffsetCm = currentVOffsetCm
        previewCanvas.isSeatedMode = isSeatedMode
    }

    private fun saveCalibration() {
        val stance = if (isSeatedMode) "seated" else "standing"
        val heightFormatted = String.format(Locale.ROOT, "%.1f", currentHeightCm)
        val ipdFormatted = String.format(Locale.ROOT, "%.1f", currentIpdMm)
        val vOffsetFormatted = String.format(Locale.ROOT, "%.1f", currentVOffsetCm)

        prefs.edit()
            .putFloat("pref_vr_height_val", currentHeightCm)
            .putString("pref_vr_height", heightFormatted)
            .putFloat("pref_vr_eye_offset_val", currentIpdMm)
            .putString("pref_vr_eye_offset", ipdFormatted)
            .putFloat("pref_vr_eye_height_offset_val", currentVOffsetCm)
            .putString("pref_vr_eye_height_offset", vOffsetFormatted)
            .putString("pref_vr_stance", stance)
            .apply()

        // Write directly to user settings config if available
        try {
            val userConfigDir = File(Constants.USER_CONFIG)
            if (!userConfigDir.exists()) {
                userConfigDir.mkdirs()
            }
            val settingsCfg = File(userConfigDir, "settings.cfg")
            if (!settingsCfg.exists()) {
                settingsCfg.createNewFile()
            }
            Writer.write(settingsCfg.absolutePath, "real height", heightFormatted)
            Writer.write(settingsCfg.absolutePath, "eye offset", String.format(Locale.ROOT, "%.3f", currentIpdMm / 1000f))
        } catch (e: Exception) {
            // Ignored - will also be written during launch in MainActivity
        }

        Toast.makeText(this, R.string.vr_calibration_saved, Toast.LENGTH_SHORT).show()
        finish()
    }
}
