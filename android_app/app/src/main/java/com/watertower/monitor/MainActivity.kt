package com.watertower.monitor

import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import com.watertower.monitor.databinding.ActivityMainBinding

/**
 * 主界面 - 简化版（仅演示模式）
 */
class MainActivity : AppCompatActivity() {
    
    private lateinit var binding: ActivityMainBinding
    private val towers = mutableListOf<TowerData>()
    private lateinit var towerAdapter: TowerAdapter
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        // 初始化 RecyclerView
        towerAdapter = TowerAdapter(
            towers,
            onPumpToggle = { tower ->
                Toast.makeText(this, "切换水泵：${tower.name}", Toast.LENGTH_SHORT).show()
            },
            onTowerClick = { tower ->
                Toast.makeText(this, "点击：${tower.name}", Toast.LENGTH_SHORT).show()
            }
        )
        binding.towerRecyclerView.layoutManager = LinearLayoutManager(this)
        binding.towerRecyclerView.adapter = towerAdapter
        
        // 加载演示数据
        loadDemoData()
        
        // 刷新按钮
        binding.fabRefresh.setOnClickListener {
            loadDemoData()
            Toast.makeText(this, "数据已刷新", Toast.LENGTH_SHORT).show()
        }
        
        Toast.makeText(this, "演示模式 - 显示模拟数据", Toast.LENGTH_LONG).show()
    }
    
    private fun loadDemoData() {
        towers.clear()
        towers.addAll(ApiClient.getDemoTowers())
        towerAdapter.notifyDataSetChanged()
    }
}
