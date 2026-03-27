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
    private var launchInProgress: Boolean = false

    override fun onCreate(savedInstanceState: Bundle?) {
        Log.d(TAG, "LauncherActivity.onCreate: starting")
        super.onCreate(savedInstanceState)
        
        PermissionHelper.getWriteExternalStoragePermission(this)
        setContentView(R.layout.launcher)
        Log.d(TAG, "LauncherActivity: set launcher layout")
        
        prefs = PreferenceManager.getDefaultSharedPreferences(this)

        // Update displayed game data path
        updateGameDataDisplay()

        // Set up button listeners
        selectDataButton = findViewById(R.id.select_data_button)
        launchGameButton = findViewById(R.id.launch_game_button)

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
            Log.d(TAG, "Launch game button clicked")
            checkStartGame()
        }

        Log.d(TAG, "LauncherActivity.onCreate: completed successfully")
    }
    
    private fun updateGameDataDisplay() {
        val gameDataPath = prefs.getString("game_files", "")!!
        val pathDisplay = findViewById<TextView>(R.id.game_data_path)
        pathDisplay.text = if (gameDataPath.isEmpty()) "(not configured)" else gameDataPath
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
        var gameFiles = ""

        val inst = GameInstaller(path)
        if (inst.check()) {
            Log.d(TAG, "setupData: path is valid")
            inst.setNomedia()
            if (!inst.convertIni(prefs.getString("pref_encoding", GameInstaller.DEFAULT_CHARSET_PREF)!!)) {
                showError(R.string.data_error_title, R.string.ini_error_message)
            } else {
                gameFiles = path
                Log.d(TAG, "setupData: configuration successful")
            }
        } else {
            Log.d(TAG, "setupData: path is NOT valid")
            showError(R.string.data_error_title, R.string.data_error_message, "https://omw.xyz.is/game.html")
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
