/*
    OpenMW VR Quest - Simple Launcher Activity
    Displays a simple button-based UI for configuring game data and launching the game
*/

package ui.activity

import android.app.AlertDialog
import android.content.DialogInterface
import android.content.Intent
import android.content.SharedPreferences
import android.os.Bundle
import android.preference.PreferenceManager
import android.util.Log
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity

import com.codekidlabs.storagechooser.StorageChooser
import com.libopenmw.openmw.R
import file.GameInstaller
import permission.PermissionHelper

private const val TAG = "LauncherActivity"

class LauncherActivity : AppCompatActivity() {
    private lateinit var prefs: SharedPreferences
    private lateinit var selectDataButton: Button
    private lateinit var launchGameButton: Button
    private lateinit var manageModsButton: Button
    private lateinit var settingsButton: Button
    private lateinit var vrCalibrationButton: Button
    private var launchInProgress: Boolean = false

    override fun onCreate(savedInstanceState: Bundle?) {
        Log.d(TAG, "LauncherActivity.onCreate: starting")
        super.onCreate(savedInstanceState)
        
        PermissionHelper.getWriteExternalStoragePermission(this)
        setContentView(R.layout.launcher)
        Log.d(TAG, "LauncherActivity: set launcher layout")
        
        prefs = PreferenceManager.getDefaultSharedPreferences(this)

        // Update displayed game data path and VR calibration
        updateGameDataDisplay()
        updateVrCalibrationDisplay()

        // Set up button listeners
        selectDataButton = findViewById(R.id.select_data_button)
        launchGameButton = findViewById(R.id.launch_game_button)
        manageModsButton = findViewById(R.id.manage_mods_button)
        settingsButton = findViewById(R.id.settings_button)
        vrCalibrationButton = findViewById(R.id.btn_vr_calibration)

        manageModsButton.setOnClickListener {
            startActivity(Intent(this, ModsActivity::class.java))
        }

        settingsButton.setOnClickListener {
            startActivity(Intent(this, MainActivity::class.java))
        }

        vrCalibrationButton.setOnClickListener {
            startActivity(Intent(this, VrCalibrationActivity::class.java))
        }

        findViewById<android.view.View>(R.id.card_vr_calibration).setOnClickListener {
            startActivity(Intent(this, VrCalibrationActivity::class.java))
        }

        selectDataButton.setOnClickListener {
            if (launchInProgress) {
                Log.d(TAG, "Select data ignored: launch already in progress")
                return@setOnClickListener
            }
            Log.d(TAG, "Select data button clicked")
            selectGameData()
        }
        
        launchGameButton.setOnClickListener {
            if (launchInProgress) {
                Log.d(TAG, "Launch game ignored: launch already in progress")
                return@setOnClickListener
            }
            launchInProgress = true
            launchGameButton.isEnabled = false
            selectDataButton.isEnabled = false
            manageModsButton.isEnabled = false
            settingsButton.isEnabled = false
            vrCalibrationButton.isEnabled = false
            Log.d(TAG, "Launch game button clicked")
            checkStartGame()
        }

        Log.d(TAG, "LauncherActivity.onCreate: completed successfully")
    }

    override fun onResume() {
        super.onResume()
        updateGameDataDisplay()
        updateVrCalibrationDisplay()
    }

    private fun updateVrCalibrationDisplay() {
        val heightCm = try {
            prefs.getFloat("pref_vr_height_val", prefs.getString("pref_vr_height", "175.0")?.toFloatOrNull() ?: 175f)
        } catch (e: Exception) {
            175f
        }

        val ipdMm = try {
            prefs.getFloat("pref_vr_eye_offset_val", prefs.getString("pref_vr_eye_offset", "64.0")?.toFloatOrNull() ?: 64f)
        } catch (e: Exception) {
            64f
        }

        val stance = prefs.getString("pref_vr_stance", "standing")
        val isSeated = stance.equals("seated", ignoreCase = true)

        val badge = findViewById<TextView>(R.id.vr_calibration_badge)
        val description = findViewById<TextView>(R.id.vr_calibration_description)

        val totalInches = (heightCm / 2.54f).toInt()
        val feet = totalInches / 12
        val inches = totalInches % 12
        val stanceLabel = if (isSeated) "Seated Mode" else "Standing Mode"

        badge?.text = "${heightCm.toInt()} cm • ${ipdMm.toInt()} mm"
        description?.text = "Height: ${heightCm.toInt()} cm (${feet}'${inches}\") • Eye IPD: ${String.format(java.util.Locale.ROOT, "%.1f", ipdMm)} mm • $stanceLabel"
    }

    private fun updateGameDataDisplay() {
        val gameDataPath = prefs.getString("game_files", "")!!
        val pathDisplay = findViewById<TextView>(R.id.game_data_path)
        val badge = findViewById<TextView>(R.id.game_data_badge)
        val statusIcon = findViewById<android.widget.ImageView>(R.id.game_data_status_icon)
        
        if (gameDataPath.isEmpty()) {
            pathDisplay.text = "(not configured - tap button below)"
            pathDisplay.setTextColor(getColor(R.color.text_secondary))
            badge.text = "Required"
            badge.setTextColor(getColor(R.color.status_warning))
            statusIcon.setImageResource(R.drawable.ic_folder_open_24)
        } else {
            pathDisplay.text = gameDataPath
            pathDisplay.setTextColor(getColor(R.color.text_primary))
            badge.text = "Configured"
            badge.setTextColor(getColor(R.color.status_ready))
            statusIcon.setImageResource(R.drawable.ic_check_circle_24)
        }
        Log.d(TAG, "updateGameDataDisplay: path='$gameDataPath'")
    }
    
    private fun selectGameData() {
        Log.d(TAG, "selectGameData: launching file browser")
        val chooser = StorageChooser.Builder()
            .withActivity(this)
            .withFragmentManager(fragmentManager)
            .withMemoryBar(true)
            .allowCustomPath(true)
            .setType(StorageChooser.DIRECTORY_CHOOSER)
            .build()

        chooser.show()

        chooser.setOnSelectListener { path ->
            Log.d(TAG, "selectGameData: user selected path='$path'")
            setupData(path)
        }
    }
    
    private fun setupData(path: String) {
        Log.d(TAG, "setupData: path='$path'")
        val cleanPath = path.trim()
        if (cleanPath.isEmpty()) return

        var gameFiles = ""
        val inst = GameInstaller(cleanPath)

        if (inst.check()) {
            Log.d(TAG, "setupData: path is valid")
            inst.setNomedia()
            inst.convertIni(prefs.getString("pref_encoding", GameInstaller.DEFAULT_CHARSET_PREF) ?: GameInstaller.DEFAULT_CHARSET_PREF)
            gameFiles = cleanPath
            val resolvedData = inst.findDataFiles()
            Toast.makeText(this, "Game data configured!\nData: $resolvedData", Toast.LENGTH_LONG).show()
        } else {
            Log.d(TAG, "setupData: path is NOT valid")
            AlertDialog.Builder(this)
                .setTitle(R.string.data_error_title)
                .setMessage("Could not find Morrowind game data in:\n\n$cleanPath\n\nPlease make sure this folder (or a subfolder) contains:\n• Morrowind.esm (or .omwgame / .esm files)\n• or a 'Data Files' folder\n\nTip: You can select the Morrowind root folder OR the 'Data Files' folder directly.")
                .setPositiveButton(android.R.string.ok, null)
                .setNeutralButton("Manual Path") { _, _ ->
                    selectGameData()
                }
                .show()
        }

        with(prefs.edit()) {
            putString("game_files", gameFiles)
            apply()
        }
        
        updateGameDataDisplay()
        val statusMsg = findViewById<TextView>(R.id.status_message)
        statusMsg.text = if (gameFiles.isEmpty()) "Invalid game data" else "Game data configured!"
    }

    private fun showError(title: Int, message: Int, url: String? = null) {
        val dialog = AlertDialog.Builder(this)
            .setTitle(title)
            .setMessage(message)
            .setPositiveButton(android.R.string.ok) { _: DialogInterface, _: Int -> }

        if (url != null) {
            dialog.setNeutralButton(R.string.dialog_howto) { _, _ ->
                openUrl(url)
            }
        }

        dialog.show()
    }

    private fun openUrl(url: String) {
        try {
            val browserIntent = Intent(Intent.ACTION_VIEW, android.net.Uri.parse(url))
            startActivity(browserIntent)
        } catch (e: Exception) {
            AlertDialog.Builder(this)
                .setTitle(R.string.no_browser_title)
                .setMessage(getString(R.string.no_browser_message, url))
                .setPositiveButton(android.R.string.ok) { _, _ -> }
                .show()
        }
    }

    private fun checkStartGame() {
        Log.d(TAG, "checkStartGame: checking if game data is configured")
        // First, check that there are game files present
        val gameFilesPath = prefs.getString("game_files", "")!!
        if (gameFilesPath.isEmpty()) {
            Log.d(TAG, "checkStartGame: no game data configured")
            launchInProgress = false
            launchGameButton.isEnabled = true
            selectDataButton.isEnabled = true
            manageModsButton.isEnabled = true
            settingsButton.isEnabled = true
            val statusMsg = findViewById<TextView>(R.id.status_message)
            statusMsg.text = "Please select game data first"
            return
        }
        
        val inst = GameInstaller(gameFilesPath)
        if (!inst.check()) {
            Log.d(TAG, "checkStartGame: game data path is no longer valid")
            launchInProgress = false
            launchGameButton.isEnabled = true
            selectDataButton.isEnabled = true
            manageModsButton.isEnabled = true
            settingsButton.isEnabled = true
            AlertDialog.Builder(this)
                .setTitle(R.string.no_data_files_title)
                .setMessage(R.string.no_data_files_message)
                .setNeutralButton(R.string.dialog_howto) { _, _ ->
                    openUrl("https://omw.xyz.is/game.html")
                }
                .setPositiveButton(android.R.string.ok) { _: DialogInterface, _: Int -> }
                .show()
            return
        }

        Log.d(TAG, "checkStartGame: game data valid, starting game")
        startGame()
    }

    private fun startGame() {
        Log.d(TAG, "startGame: routing through VrEntryActivity for full prep")
        val intent = Intent(this, VrEntryActivity::class.java)
        intent.putExtra(VrEntryActivity.EXTRA_AUTO_START_GAME, true)
        startActivity(intent)
        finish()
    }
}
