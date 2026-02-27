package com.watertower.monitor

/**
 * 水塔数据模型
 */
data class TowerData(
    val id: Int,
    val name: String,
    val waterLevel: Int,        // 水位百分比 0-100
    val pumpOn: Boolean,        // 水泵状态
    val online: Boolean,        // 在线状态
    val lastUpdate: Long,       // 最后更新时间戳
    val autoMode: Boolean = true,
    val lowWaterAlarm: Boolean = false,  // 缺水报警
    val overflowAlarm: Boolean = false   // 溢水报警
)

/**
 * 系统状态
 */
data class SystemStatus(
    val autoMode: Boolean,
    val totalTowers: Int,
    val onlineTowers: Int,
    val alarmCount: Int,
    val wellWaterOk: Boolean
)

/**
 * 控制命令
 */
data class ControlCommand(
    val towerId: Int,
    val pumpOn: Boolean,
    val autoMode: Boolean? = null
)

/**
 * 历史记录
 */
data class HistoryRecord(
    val towerId: Int,
    val timestamp: Long,
    val waterLevel: Int,
    val pumpStatus: Boolean
)
