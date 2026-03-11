package com.watertower.monitor

import android.content.Intent
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.watertower.monitor.databinding.ActivityMainBinding

/**
 * 主界面 - 显示所有水塔状态
 */
class MainActivity : AppCompatActivity() {
    
    private lateinit var binding: ActivityMainBinding
    private lateinit var towerAdapter: TowerAdapter
    private val handler = Handler(Looper.getMainLooper())
    
    private var demoMode = true
    private var towers = mutableListOf<TowerData>()
    
    // 自动刷新间隔 (5 秒)
    private val refreshInterval = 5000L
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        // 初始化视图
        initViews()
        
        // 加载演示数据
        loadDemoData()
        
        // 设置点击事件
        setupClickListeners()
        
        // 开始自动刷新
        startAutoRefresh()
    }
    
    override fun onDestroy() {
        super.onDestroy()
        handler.removeCallbacksAndMessages(null)
    }
    
    /**
     * 初始化视图
     */
    private fun initViews() {
        // 设置 RecyclerView
        towerAdapter = TowerAdapter(
            towers = towers,
            onPumpToggle = { tower -> togglePump(tower) },
            onTowerClick = { tower -> showTowerDetail(tower) }
        )
        
        binding.towerRecyclerView.layoutManager = LinearLayoutManager(this)
        binding.towerRecyclerView.adapter = towerAdapter
        
        // 初始状态
        updateSystemStatus()
    }
    
    /**
     * 加载演示数据
     */
    private fun loadDemoData() {
        towers.clear()
        towers.addAll(ApiClient.getDemoTowers())
        towerAdapter.notifyDataSetChanged()
        updateSystemStatus()
    }
    
    /**
     * 更新系统状态
     */
    private fun updateSystemStatus() {
        val onlineCount = towers.count { it.online }
        val alarmCount = towers.count { it.lowWaterAlarm || it.overflowAlarm }
        
        binding.onlineCount.text = "$onlineCount/${towers.size}"
        binding.wellWaterStatus.text = if (ApiClient.demoMode) "正常" else "未知"
        binding.wellWaterStatus.setTextColor(
            getColor(if (ApiClient.demoMode) R.color.success else R.color.text_secondary)
        )
        binding.modeText.text = if (towers.firstOrNull()?.autoMode != false) "自动模式" else "手动模式"
    }
    
    /**
     * 设置点击事件
     */
    private fun setupClickListeners() {
        // 刷新按钮
        binding.fabRefresh.setOnClickListener {
            refreshData()
        }
        
        // 底部导航
        binding.btnHome.setOnClickListener {
            // 已在首页
            Toast.makeText(this, "已在首页", Toast.LENGTH_SHORT).show()
        }
        
        binding.btnHistory.setOnClickListener {
            startActivity(Intent(this, HistoryActivity::class.java))
        }
        
        binding.btnSettings.setOnClickListener {
            startActivity(Intent(this, SettingsActivity::class.java))
        }
        
        // 连接状态点击
        binding.connectionStatus.setOnClickListener {
            showConnectionDialog()
        }
    }
    
    /**
     * 切换水泵
     */
    private fun togglePump(tower: TowerData) {
        val newStatus = !tower.pumpOn
        
        // 演示模式：直接更新
        val index = towers.indexOfFirst { it.id == tower.id }
        if (index >= 0) {
            towers[index] = tower.copy(pumpOn = newStatus)
            towerAdapter.notifyItemChanged(index)
            
            Toast.makeText(
                this,
                "${tower.name} 水泵已${if (newStatus) "开启" else "关闭"}",
                Toast.LENGTH_SHORT
            ).show()
        }
    }
    
    /**
     * 显示水塔详情
     */
    private fun showTowerDetail(tower: TowerData) {
        val statusText = buildString {
            appendLine("水塔名称：${tower.name}")
            appendLine("当前水位：${tower.waterLevel}%")
            appendLine("水泵状态：${if (tower.pumpOn) "运行中" else "已停止"}")
            appendLine("在线状态：${if (tower.online) "在线" else "离线"}")
            appendLine("运行模式：${if (tower.autoMode) "自动" else "手动"}")
            
            if (tower.lowWaterAlarm) appendLine("⚠️ 缺水报警")
            if (tower.overflowAlarm) appendLine("⚠️ 溢水报警")
        }
        
        AlertDialog.Builder(this)
            .setTitle(tower.name)
            .setMessage(statusText)
            .setPositiveButton("控制水泵") { _, _ ->
                togglePump(tower)
            }
            .setNegativeButton("查看历史") { _, _ ->
                // TODO: 跳转到历史页面
                Toast.makeText(this, "查看 ${tower.name} 历史记录", Toast.LENGTH_SHORT).show()
            }
            .setNeutralButton("关闭", null)
            .show()
    }
    
    /**
     * 显示连接设置对话框
     */
    private fun showConnectionDialog() {
        val builder = AlertDialog.Builder(this)
        builder.setTitle("连接设置")
        
        val items = arrayOf("演示模式 (当前)", "连接设备")
        builder.setItems(items) { _, which ->
            when (which) {
                0 -> {
                    demoMode = true
                    ApiClient.demoMode = true
                    loadDemoData()
                    Toast.makeText(this, "已切换到演示模式", Toast.LENGTH_SHORT).show()
                }
                1 -> {
                    showServerAddressDialog()
                }
            }
        }
        builder.show()
    }
    
    /**
     * 显示服务器地址输入对话框
     */
    private fun showServerAddressDialog() {
        val builder = AlertDialog.Builder(this)
        builder.setTitle("输入服务器地址")
        
        val editText = android.widget.EditText(this)
        editText.setText(ApiClient.getBaseUrl())
        editText.hint = "例如：http://192.168.4.1"
        builder.setView(editText)
        
        builder.setPositiveButton("连接") { _, _ ->
            val url = editText.text.toString().trim()
            if (url.isNotEmpty()) {
                ApiClient.setBaseUrl(url)
                ApiClient.demoMode = false
                demoMode = false
                refreshData()
                Toast.makeText(this, "正在连接 $url", Toast.LENGTH_SHORT).show()
            }
        }
        builder.setNegativeButton("取消", null)
        builder.show()
    }
    
    /**
     * 刷新数据
     */
    private fun refreshData() {
        binding.fabRefresh.isEnabled = false
        
        if (demoMode) {
            // 演示模式：刷新演示数据
            handler.postDelayed({
                loadDemoData()
                binding.fabRefresh.isEnabled = true
                Toast.makeText(this, "数据已刷新", Toast.LENGTH_SHORT).show()
            }, 500)
        } else {
            // 实际模式：调用 API
            ApiClient.api.getSystemStatus().enqueue(
                object : retrofit2.Callback<List<TowerData>> {
                    override fun onResponse(
                        call: retrofit2.Call<List<TowerData>>,
                        response: retrofit2.Response<List<TowerData>>
                    ) {
                        binding.fabRefresh.isEnabled = true
                        if (response.isSuccessful && response.body() != null) {
                            towers.clear()
                            towers.addAll(response.body()!!)
                            towerAdapter.notifyDataSetChanged()
                            updateSystemStatus()
                            Toast.makeText(this@MainActivity, "数据已更新", Toast.LENGTH_SHORT).show()
                        } else {
                            Toast.makeText(
                                this@MainActivity,
                                "获取数据失败：${response.code()}",
                                Toast.LENGTH_SHORT
                            ).show()
                        }
                    }
                    
                    override fun onFailure(call: retrofit2.Call<List<TowerData>>, t: Throwable) {
                        binding.fabRefresh.isEnabled = true
                        Toast.makeText(
                            this@MainActivity,
                            "网络错误：${t.message}",
                            Toast.LENGTH_SHORT
                        ).show()
                    }
                }
            )
        }
    }
    
    /**
     * 开始自动刷新
     */
    private fun startAutoRefresh() {
        handler.postDelayed(object : Runnable {
            override fun run() {
                if (!demoMode) {
                    refreshData()
                }
                handler.postDelayed(this, refreshInterval)
            }
        }, refreshInterval)
    }
}
