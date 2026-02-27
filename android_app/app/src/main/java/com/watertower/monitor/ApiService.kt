package com.watertower.monitor

import retrofit2.http.*

/**
 * ESP8266 API 接口
 */
interface WaterTowerApi {
    
    // 获取系统状态
    @GET("api/status")
    suspend fun getSystemStatus(): SystemStatus
    
    // 获取所有水塔数据
    @GET("api/towers")
    suspend fun getAllTowers(): List<TowerData>
    
    // 获取单个水塔数据
    @GET("api/tower/{id}")
    suspend fun getTower(@Path("id") towerId: Int): TowerData
    
    // 控制水泵
    @POST("api/pump")
    suspend fun controlPump(@Body command: ControlCommand): Map<String, Any>
    
    // 切换模式
    @POST("api/mode")
    suspend fun setMode(@Body mode: Map<String, Boolean>): Map<String, Any>
    
    // 获取历史记录
    @GET("api/history")
    suspend fun getHistory(
        @Query("towerId") towerId: Int,
        @Query("hours") hours: Int = 24
    ): List<HistoryRecord>
}
