package com.watertower.monitor

import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.watertower.monitor.databinding.ActivitySettingsBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class SettingsActivity : AppCompatActivity() {
    
    private lateinit var binding: ActivitySettingsBinding
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivitySettingsBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        setSupportActionBar(binding.toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        
        binding.toolbar.setNavigationOnClickListener {
            finish()
        }
        
        // 加载当前配置
        binding.editServerAddress.setText(ApiClient.getBaseUrl())
        
        // 保存按钮
        binding.btnSave.setOnClickListener {
            saveSettings()
        }
        
        // 测试连接按钮
        binding.btnTestConnection.setOnClickListener {
            testConnection()
        }
    }
    
    private fun saveSettings() {
        val url = binding.editServerAddress.text.toString().trim()
        if (url.isEmpty()) {
            Toast.makeText(this, "请输入服务器地址", Toast.LENGTH_SHORT).show()
            return
        }
        
        ApiClient.setBaseUrl(url)
        Toast.makeText(this, "设置已保存", Toast.LENGTH_SHORT).show()
        finish()
    }
    
    private fun testConnection() {
        val url = binding.editServerAddress.text.toString().trim()
        if (url.isEmpty()) {
            Toast.makeText(this, "请输入服务器地址", Toast.LENGTH_SHORT).show()
            return
        }
        
        lifecycleScope.launch {
            try {
                withContext(Dispatchers.IO) {
                    ApiClient.setBaseUrl(url)
                    ApiClient.api.getSystemStatus()
                }
                withContext(Dispatchers.Main) {
                    Toast.makeText(this@SettingsActivity, "连接成功", Toast.LENGTH_SHORT).show()
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    Toast.makeText(
                        this@SettingsActivity,
                        "连接失败：${e.message}",
                        Toast.LENGTH_LONG
                    ).show()
                }
            }
        }
    }
}
