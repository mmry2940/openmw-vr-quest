package com.codekidlabs.storagechooser

import android.app.Activity
import android.app.AlertDialog
import android.os.Environment
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.Toast
import java.io.File

class StorageChooser private constructor(
    private val activity: Activity?,
    private val allowCustomPath: Boolean,
    private val type: String
) {
    private var selectListener: ((String) -> Unit)? = null

    fun show() {
        if (activity == null || activity.isFinishing) return

        val defaultRoot = Environment.getExternalStorageDirectory() ?: activity.filesDir
        var currentDir = if (defaultRoot.exists()) defaultRoot else activity.filesDir

        showDirectoryPicker(currentDir)
    }

    private fun showDirectoryPicker(dir: File) {
        if (activity == null || activity.isFinishing) return

        val files = try {
            dir.listFiles()?.filter { it.isDirectory && !it.name.startsWith(".") }?.sortedBy { it.name.lowercase() }
                ?: emptyList()
        } catch (e: Exception) {
            emptyList()
        }

        val itemNames = ArrayList<String>()
        val itemFiles = ArrayList<File>()

        if (dir.parentFile != null && dir.absolutePath != Environment.getExternalStorageDirectory().parent) {
            itemNames.add("📁 .. (Parent Directory)")
            itemFiles.add(dir.parentFile!!)
        }

        for (f in files) {
            itemNames.add("📁 " + f.name)
            itemFiles.add(f)
        }

        val builder = AlertDialog.Builder(activity)
        builder.setTitle("Select Folder: ${dir.absolutePath}")

        builder.setItems(itemNames.toTypedArray()) { _, which ->
            val target = itemFiles[which]
            showDirectoryPicker(target)
        }

        builder.setPositiveButton("Select This Folder") { _, _ ->
            selectListener?.invoke(dir.absolutePath)
        }

        builder.setNeutralButton("Enter Path") { _, _ ->
            showManualPathDialog(dir.absolutePath)
        }

        builder.setNegativeButton("Cancel", null)
        builder.show()
    }

    private fun showManualPathDialog(initialPath: String) {
        if (activity == null || activity.isFinishing) return

        val input = EditText(activity)
        input.setText(initialPath)
        val container = LinearLayout(activity)
        container.setPadding(40, 20, 40, 20)
        container.addView(input, LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT)

        AlertDialog.Builder(activity)
            .setTitle("Enter Path")
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
