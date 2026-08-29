package com.codekidlabs.storagechooser

import android.app.Activity
import android.app.AlertDialog
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Environment
import android.provider.Settings
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.Toast
import java.io.File
import java.util.Locale

class StorageChooser private constructor(
    private val activity: Activity?,
    private val allowCustomPath: Boolean,
    private val type: String
) {
    private var selectListener: ((String) -> Unit)? = null

    fun show() {
        if (activity == null || activity.isFinishing) return

        // Verify storage permissions on Android 11+
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                AlertDialog.Builder(activity)
                    .setTitle("Storage Permission Required")
                    .setMessage("To access Morrowind game files on your device or VR headset, OpenMW VR requires All Files Access permission.\n\nPlease enable 'Allow management of all files' in settings.")
                    .setPositiveButton("Grant Access") { _, _ ->
                        try {
                            val intent = Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION).apply {
                                data = Uri.parse("package:${activity.packageName}")
                            }
                            activity.startActivity(intent)
                        } catch (e: Exception) {
                            val intent = Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION)
                            activity.startActivity(intent)
                        }
                    }
                    .setNegativeButton("Cancel", null)
                    .show()
                return
            }
        }

        val defaultRoot = Environment.getExternalStorageDirectory() ?: activity.filesDir
        val currentDir = if (defaultRoot.exists()) defaultRoot else activity.filesDir

        showDirectoryPicker(currentDir)
    }

    private fun hasGameAssets(dir: File): Boolean {
        if (!dir.exists() || !dir.isDirectory) return false
        val files = try {
            dir.listFiles()
        } catch (e: Exception) {
            null
        } ?: return false

        return files.any { f ->
            val name = f.name.lowercase(Locale.ROOT)
            name == "morrowind.esm" || name == "morrowind.ini" || name == "data files" ||
            name.endsWith(".esm") || name.endsWith(".bsa") || name.endsWith(".omwgame")
        }
    }

    private fun showDirectoryPicker(dir: File) {
        if (activity == null || activity.isFinishing) return

        val files = try {
            dir.listFiles()?.filter { it.isDirectory && !it.name.startsWith(".") }?.sortedBy { it.name.lowercase(Locale.ROOT) }
                ?: emptyList()
        } catch (e: Exception) {
            emptyList()
        }

        val isGameDir = hasGameAssets(dir)

        val itemNames = ArrayList<String>()
        val itemFiles = ArrayList<File>()

        if (dir.parentFile != null && dir.absolutePath != Environment.getExternalStorageDirectory().parent) {
            itemNames.add("📁 ⬆ .. (Up to Parent Folder)")
            itemFiles.add(dir.parentFile!!)
        }

        for (f in files) {
            val containsGame = hasGameAssets(f)
            val prefix = if (containsGame) "🎮 [Game Files] 📁 " else "📁 "
            itemNames.add(prefix + f.name)
            itemFiles.add(f)
        }

        val title = if (isGameDir) {
            "✓ Game Files Found!\n${dir.absolutePath}"
        } else {
            "Select Folder:\n${dir.absolutePath}"
        }

        val builder = AlertDialog.Builder(activity)
        builder.setTitle(title)

        builder.setItems(itemNames.toTypedArray()) { _, which ->
            val target = itemFiles[which]
            showDirectoryPicker(target)
        }

        builder.setPositiveButton("Select This Folder") { _, _ ->
            selectListener?.invoke(dir.absolutePath)
        }

        builder.setNeutralButton("Shortcuts / Enter Path") { _, _ ->
            showShortcutsDialog(dir.absolutePath)
        }

        builder.setNegativeButton("Cancel", null)
        builder.show()
    }

    private fun showShortcutsDialog(currentPath: String) {
        if (activity == null || activity.isFinishing) return

        val shortcuts = arrayListOf(
            "📂 /sdcard (Internal Storage)" to Environment.getExternalStorageDirectory().absolutePath,
            "📂 /sdcard/Morrowind" to File(Environment.getExternalStorageDirectory(), "Morrowind").absolutePath,
            "📂 /sdcard/Download" to Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS).absolutePath,
            "📂 /sdcard/openmw" to File(Environment.getExternalStorageDirectory(), "openmw").absolutePath,
            "📂 App Private Files" to (activity.getExternalFilesDir(null)?.absolutePath ?: activity.filesDir.absolutePath),
            "✏ Type Custom Path..." to ""
        )

        val names = shortcuts.map { it.first }.toTypedArray()

        AlertDialog.Builder(activity)
            .setTitle("Quick Folder Jump")
            .setItems(names) { _, which ->
                val chosen = shortcuts[which].second
                if (chosen.isEmpty()) {
                    showManualPathDialog(currentPath)
                } else {
                    val file = File(chosen)
                    if (!file.exists()) {
                        file.mkdirs()
                    }
                    showDirectoryPicker(file)
                }
            }
            .setNegativeButton("Back") { _, _ ->
                showDirectoryPicker(File(currentPath))
            }
            .show()
    }

    private fun showManualPathDialog(initialPath: String) {
        if (activity == null || activity.isFinishing) return

        val input = EditText(activity)
        input.setText(initialPath)
        input.hint = "/sdcard/Morrowind or /sdcard/Download/..."
        val container = LinearLayout(activity)
        container.setPadding(40, 20, 40, 20)
        container.addView(input, LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT)

        AlertDialog.Builder(activity)
            .setTitle("Enter Path to Game Data")
            .setView(container)
            .setPositiveButton("OK") { _, _ ->
                val path = input.text.toString().trim()
                if (path.isNotEmpty()) {
                    selectListener?.invoke(path)
                }
            }
            .setNegativeButton("Cancel", null)
            .show()
    }

    fun setOnSelectListener(listener: OnSelectListener) {
        this.selectListener = { listener.onSelect(it) }
    }

    fun setOnSelectListener(listener: (String) -> Unit) {
        this.selectListener = listener
    }

    fun interface OnSelectListener {
        fun onSelect(path: String)
    }

    class Builder {
        private var activity: Activity? = null
        private var allowCustomPath: Boolean = true
        private var type: String = DIRECTORY_CHOOSER

        fun withActivity(activity: Activity?): Builder {
            this.activity = activity
            return this
        }

        fun withFragmentManager(fm: Any?): Builder {
            return this
        }

        fun withMemoryBar(bar: Boolean): Builder {
            return this
        }

        fun allowCustomPath(allow: Boolean): Builder {
            this.allowCustomPath = allow
            return this
        }

        fun setType(type: String): Builder {
            this.type = type
            return this
        }

        fun build(): StorageChooser {
            return StorageChooser(activity, allowCustomPath, type)
        }
    }

    companion object {
        const val DIRECTORY_CHOOSER = "dir"
        const val FILE_PICKER = "file"
        const val NONE = "none"
    }
}
