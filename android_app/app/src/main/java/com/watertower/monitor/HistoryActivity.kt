package com.watertower.monitor

import android.app.DatePickerDialog
import android.graphics.Color
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.github.mikephil.charting.charts.LineChart
import com.github.mikephil.charting.components.XAxis
import com.github.mikephil.charting.data.Entry
import com.github.mikephil.charting.data.LineData
import com.github.mikephil.charting.data.LineDataSet
import com.github.mikephil.charting.formatter.ValueFormatter
import com.watertower.monitor.databinding.ActivityHistoryBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.text.SimpleDateFormat
import java.util.*

class HistoryActivity : AppCompatActivity() {
    
    private lateinit var binding: ActivityHistoryBinding
    private var selectedDate: Calendar = Calendar.getInstance()
    private var selectedTowerId: Int = 1 // 默认水塔 1
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityHistoryBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        setSupportActionBar(binding.toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        
        binding.toolbar.setNavigationOnClickListener {
            finish()
        }
        
        // 日期选择按钮
        binding.btnSelectDate.setOnClickListener {
            showDatePicker()
        }
        
        // 初始加载今天的数据
        loadHistoryData()
    }
    
    private fun showDatePicker() {
        DatePickerDialog(
            this,
            { _, year, month, dayOfMonth ->
                selectedDate.set(year, month, dayOfMonth)
                updateDateDisplay()
                loadHistoryData()
            },
            selectedDate.get(Calendar.YEAR),
            selectedDate.get(Calendar.MONTH),
            selectedDate.get(Calendar.DAY_OF_MONTH)
        ).apply {
            datePicker.maxDate = System.currentTimeMillis()
        }.show()
    }
    
    private fun updateDateDisplay() {
        val sdf = SimpleDateFormat("yyyy 年 MM 月 dd 日", Locale.CHINA)
        binding.textSelectedDate.text = sdf.format(selectedDate.time)
    }
    
    private fun loadHistoryData() {
        binding.textSelectedDate.text = "加载中..."
        
        lifecycleScope.launch {
            try {
                // 计算从选中日期到现在的小时数
                val now = Calendar.getInstance()
                val hours = ((now.timeInMillis - selectedDate.timeInMillis) / (1000 * 60 * 60)).toInt() + 1
                
                val history = withContext(Dispatchers.IO) {
                    ApiClient.api.getHistory(towerId = selectedTowerId, hours = hours)
                }
                
                withContext(Dispatchers.Main) {
                    updateDateDisplay()
                    displayChart(history)
                    displayStatistics(history)
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    binding.textSelectedDate.text = "加载失败：${e.message}"
                    // 显示模拟数据用于演示
                    displayMockData()
                }
            }
        }
    }
    
    private fun displayChart(history: List<HistoryRecord>) {
        val chart: LineChart = binding.chartWaterLevel
        
        if (history.isEmpty()) {
            chart.clear()
            chart.setNoDataText("暂无历史数据")
            return
        }
        
        // 准备数据
        val entries = history.mapIndexed { index, record ->
            Entry(index.toFloat(), record.waterLevel.toFloat())
        }
        
        // 创建数据集
        val dataSet = LineDataSet(entries, "水位").apply {
            color = Color.parseColor("#2196F3")
            lineWidth = 2f
            setDrawCircles(true)
            setDrawCircleHole(true)
            circleColor = Color.parseColor("#2196F3")
            circleRadius = 4f
            setDrawFilled(true)
            fillColor = Color.parseColor("#64B5F6")
            fillAlpha = 100
            setDrawValues(false)
            mode = LineDataSet.Mode.CUBIC_BEZIER
            cubicIntensity = 0.2f
        }
        
        // 设置数据
        val lineData = LineData(dataSet)
        chart.data = lineData
        
        // 配置图表
        chart.apply {
            description.isEnabled = false
            setTouchEnabled(true)
            isDragEnabled = true
            setScaleEnabled(true)
            setPinchZoom(true)
            
            // X 轴配置
            xAxis.apply {
                position = XAxis.XAxisPosition.BOTTOM
                setDrawGridLines(false)
                setDrawAxisLine(true)
                axisLineColor = Color.GRAY
                valueFormatter = object : ValueFormatter() {
                    override fun getFormattedValue(value: Float): String {
                        val index = value.toInt()
                        if (index < history.size) {
                            val time = Date(history[index].timestamp)
                            val sdf = SimpleDateFormat("HH:mm", Locale.CHINA)
                            return sdf.format(time)
                        }
                        return ""
                    }
                }
                setLabelCount(6, true)
                setLabelRotationAngle(-45f)
            }
            
            // 左侧 Y 轴配置
            axisLeft.apply {
                setDrawGridLines(true)
                gridColor = Color.parseColor("#E0E0E0")
                axisMinimum = 0f
                axisMaximum = 100f
                setLabelCount(6, true)
                valueFormatter = object : ValueFormatter() {
                    override fun getFormattedValue(value: Float): String {
                        return "${value.toInt()}%"
                    }
                }
            }
            
            // 右侧 Y 轴
            axisRight.isEnabled = false
            
            // 图例
            legend.apply {
                isEnabled = true
                textColor = Color.BLACK
            }
            
            invalidate() // 刷新图表
        }
    }
    
    private fun displayStatistics(history: List<HistoryRecord>) {
        if (history.isEmpty()) {
            binding.textMaxLevel.text = "--"
            binding.textMinLevel.text = "--"
            binding.textAvgLevel.text = "--"
            return
        }
        
        val levels = history.map { it.waterLevel }
        val maxLevel = levels.maxOrNull() ?: 0
        val minLevel = levels.minOrNull() ?: 0
        val avgLevel = levels.average().toInt()
        
        binding.textMaxLevel.text = "$maxLevel%"
        binding.textMinLevel.text = "$minLevel%"
        binding.textAvgLevel.text = "$avgLevel%"
    }
    
    private fun displayMockData() {
        // 生成模拟数据用于演示
        val mockData = mutableListOf<HistoryRecord>()
        val now = System.currentTimeMillis()
        val random = Random()
        
        for (i in 0 until 24) {
            mockData.add(
                HistoryRecord(
                    towerId = selectedTowerId,
                    timestamp = now - (23 - i) * 3600000,
                    waterLevel = 40 + random.nextInt(40), // 40-80% 随机
                    pumpStatus = random.nextBoolean()
                )
            )
        }
        
        displayChart(mockData)
        displayStatistics(mockData)
    }
}
