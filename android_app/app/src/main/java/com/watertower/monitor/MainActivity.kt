package com.watertower.monitor

import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import com.watertower.monitor.databinding.ActivityMainBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class MainActivity : AppCompatActivity() {
    
    private lateinit var binding: ActivityMainBinding
    private var autoRefreshJob: Job? = null
    private val towerAdapter = TowerAdapter()
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        setSupportActionBar(binding.toolbar)
        
        // 设置 RecyclerView
        binding.recyclerTowers.layoutManager = LinearLayoutManager(this)
        binding.recyclerTowers.adapter = towerAdapter
        
        // 模式切换按钮
        binding.btnToggleMode.setOnClickListener {
            toggleMode()
        }
        
        // 刷新按钮
        binding.btnRefresh.setOnClickListener {
            refreshData()
        }
        
        // 水塔点击事件
        towerAdapter.onPumpClick = { tower ->
            showPumpControlDialog(tower)
        }
        
        // 开始自动刷新
        startAutoRefresh()
    }
    
    override fun onResume() {
        super.onResume()
        refreshData()
        startAutoRefresh()
    }
    
    override fun onPause() {
        super.onPause()
        stopAutoRefresh()
    }
    
    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.main_menu, menu)
        return true
    }
    
    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_settings -> {
                startActivity(android.content.Intent(this, SettingsActivity::class.java))
                true
            }
            R.id.action_history -> {
                startActivity(android.content.Intent(this, HistoryActivity::class.java))
                true
            }
            R.id.action_refresh -> {
                refreshData()
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }
    
    private fun startAutoRefresh() {
        autoRefreshJob?.cancel()
        autoRefreshJob = lifecycleScope.launch {
            while (true) {
                delay(5000) // 5 秒刷新一次
                withContext(Dispatchers.Main) {
                    refreshData()
                }
            }
        }
    }
    
    private fun stopAutoRefresh() {
        autoRefreshJob?.cancel()
        autoRefreshJob = null
    }
    
    private fun refreshData() {
        binding.swipeRefresh.isRefreshing = true
        
        lifecycleScope.launch {
            try {
                val towers = withContext(Dispatchers.IO) {
                    ApiClient.api.getAllTowers()
                }
                
                withContext(Dispatchers.Main) {
                    towerAdapter.submitList(towers)
                    updateStatus(towers)
                    binding.swipeRefresh.isRefreshing = false
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    binding.swipeRefresh.isRefreshing = false
                    Toast.makeText(
                        this@MainActivity,
                        "连接失败：${e.message}",
                        Toast.LENGTH_SHORT
                    ).show()
                }
            }
        }
    }
    
    private fun updateStatus(towers: List<TowerData>) {
        val onlineCount = towers.count { it.online }
        val alarmCount = towers.count { it.lowWaterAlarm || it.overflowAlarm }
        
        binding.textStatus.text = "在线：$onlineCount/${towers.size}  报警：$alarmCount"
        
        // 更新模式显示
        val modeText = if (towers.firstOrNull()?.autoMode == true) "自动" else "手动"
        binding.btnToggleMode.text = "模式：$modeText"
    }
    
    private fun toggleMode() {
        lifecycleScope.launch {
            try {
                ApiClient.api.setMode(mapOf("autoMode" to true))
                Toast.makeText(this@MainActivity, "模式已切换", Toast.LENGTH_SHORT).show()
                refreshData()
            } catch (e: Exception) {
                Toast.makeText(this@MainActivity, "切换失败", Toast.LENGTH_SHORT).show()
            }
        }
    }
    
    private fun showPumpControlDialog(tower: TowerData) {
        if (!tower.online) {
            Toast.makeText(this, "设备离线", Toast.LENGTH_SHORT).show()
            return
        }
        
        AlertDialog.Builder(this)
            .setTitle("${tower.name} - 水泵控制")
            .setMessage("当前水位：${tower.waterLevel}%\n\n是否${if (tower.pumpOn) "停止" else "开启"}水泵？")
            .setPositiveButton("确认") { _, _ ->
                controlPump(tower, !tower.pumpOn)
            }
            .setNegativeButton("取消", null)
            .show()
    }
    
    private fun controlPump(tower: TowerData, turnOn: Boolean) {
        lifecycleScope.launch {
            try {
                withContext(Dispatchers.IO) {
                    ApiClient.api.controlPump(
                        ControlCommand(
                            towerId = tower.id,
                            pumpOn = turnOn,
                            autoMode = false
                        )
                    )
                }
                withContext(Dispatchers.Main) {
                    Toast.makeText(
                        this@MainActivity,
                        "水泵已${if (turnOn) "开启" else "停止"}",
                        Toast.LENGTH_SHORT
                    ).show()
                    refreshData()
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    Toast.makeText(this@MainActivity, "控制失败", Toast.LENGTH_SHORT).show()
                }
            }
        }
    }
}
