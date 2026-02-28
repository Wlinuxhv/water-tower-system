package com.watertower.monitor

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.watertower.monitor.databinding.ActivityHistoryBinding

class HistoryActivity : AppCompatActivity() {
    
    private lateinit var binding: ActivityHistoryBinding
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityHistoryBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        setSupportActionBar(binding.toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        
        binding.toolbar.setNavigationOnClickListener {
            finish()
        }
        
        // 显示提示信息
        binding.textSelectedDate.text = "历史记录功能开发中..."
        binding.textMaxLevel.text = "--"
        binding.textMinLevel.text = "--"
        binding.textAvgLevel.text = "--"
    }
}
