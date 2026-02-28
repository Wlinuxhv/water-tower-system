package com.watertower.monitor

import okhttp3.OkHttpClient
import okhttp3.logging.HttpLoggingInterceptor
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import java.util.concurrent.TimeUnit

/**
 * API 服务管理器
 */
object ApiClient {
    
    private var retrofit: Retrofit? = null
    private var baseUrl: String = "http://192.168.4.1" // ESP8266 默认 AP IP
    var demoMode: Boolean = false // 演示模式开关
    
    fun setBaseUrl(url: String) {
        baseUrl = url
        retrofit = null // 重置 retrofit
    }
    
    fun getBaseUrl(): String = baseUrl
    
    private fun getRetrofit(): Retrofit {
        if (retrofit == null) {
            val logging = HttpLoggingInterceptor().apply {
                level = HttpLoggingInterceptor.Level.BODY
            }
            
            val client = OkHttpClient.Builder()
                .addInterceptor(logging)
                .connectTimeout(10, TimeUnit.SECONDS)
                .readTimeout(30, TimeUnit.SECONDS)
                .writeTimeout(30, TimeUnit.SECONDS)
                .build()
            
            retrofit = Retrofit.Builder()
                .baseUrl(baseUrl)
                .client(client)
                .addConverterFactory(GsonConverterFactory.create())
                .build()
        }
        return retrofit!!
    }
    
    val api: WaterTowerApi by lazy {
        getRetrofit().create(WaterTowerApi::class.java)
    }
    
    /**
     * 生成演示数据
     */
    fun getDemoTowers(): List<TowerData> {
        val now = System.currentTimeMillis()
        return listOf(
            TowerData(1, "1 号水塔", 65, false, true, now, true, false, false),
            TowerData(2, "2 号水塔", 42, true, true, now, true, false, false),
            TowerData(3, "3 号水塔", 78, false, true, now, true, false, false),
            TowerData(4, "4 号水塔", 23, false, true, now, true, true, false),
            TowerData(5, "5 号水塔", 89, false, false, now - 1800000, false, false, false)
        )
    }
    
    /**
     * 生成演示历史数据（24 小时）
     */
    fun getDemoHistory(towerId: Int, hours: Int): List<HistoryRecord> {
        val records = mutableListOf<HistoryRecord>()
        val now = System.currentTimeMillis()
        val random = kotlin.random.Random(towerId) // 固定种子，数据稳定
        
        for (i in 0 until hours) {
            val timestamp = now - (hours - i - 1) * 3600000L
            // 生成有波动的水位数据
            val baseLevel = when (towerId) {
                1 -> 60
                2 -> 45
                3 -> 75
                4 -> 30
                else -> 85
            }
            val variation = random.nextInt(-15, 15)
            val waterLevel = (baseLevel + variation).coerceIn(0, 100)
            val pumpOn = waterLevel < 30 || waterLevel > 80
            
            records.add(HistoryRecord(towerId, timestamp, waterLevel, pumpOn))
        }
        
        return records
    }
}
