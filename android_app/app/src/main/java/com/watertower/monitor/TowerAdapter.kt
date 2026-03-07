package com.watertower.monitor

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ProgressBar
import android.widget.TextView
import android.widget.Button
import androidx.cardview.widget.CardView
import androidx.recyclerview.widget.RecyclerView
import java.text.SimpleDateFormat
import java.util.*

/**
 * 水塔列表适配器
 */
class TowerAdapter(
    private var towers: List<TowerData>,
    private val onPumpToggle: (TowerData) -> Unit,
    private val onTowerClick: (TowerData) -> Unit
) : RecyclerView.Adapter<TowerAdapter.TowerViewHolder>() {
    
    inner class TowerViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        private val cardName: TextView = itemView.findViewById(R.id.cardName)
        private val textLevel: TextView = itemView.findViewById(R.id.textLevel)
        private val progressLevel: ProgressBar = itemView.findViewById(R.id.progressLevel)
        private val textStatus: TextView = itemView.findViewById(R.id.textStatus)
        private val textAlarm: TextView = itemView.findViewById(R.id.textAlarm)
        private val btnPump: Button = itemView.findViewById(R.id.btnPump)
        private val textUpdate: TextView = itemView.findViewById(R.id.textUpdate)
        private val card: CardView = itemView.findViewById(R.id.towerCard)
        
        fun bind(tower: TowerData) {
            // 水塔名称
            cardName.text = "${tower.name}"
            
            // 水位
            textLevel.text = "${tower.waterLevel}%"
            progressLevel.progress = tower.waterLevel
            
            // 水位颜色
            val progressColor = when {
                tower.waterLevel < 20 -> itemView.context.getColor(R.color.error)
                tower.waterLevel > 90 -> itemView.context.getColor(R.color.warning)
                else -> itemView.context.getColor(R.color.success)
            }
            progressLevel.progressTintList = android.content.res.ColorStateList.valueOf(progressColor)
            
            // 在线状态
            if (tower.online) {
                textStatus.text = "● 在线"
                textStatus.setTextColor(itemView.context.getColor(R.color.success))
            } else {
                textStatus.text = "○ 离线"
                textStatus.setTextColor(itemView.context.getColor(R.color.text_secondary))
            }
            
            // 报警状态
            when {
                tower.lowWaterAlarm -> {
                    textAlarm.text = "⚠️ 缺水报警"
                    textAlarm.visibility = View.VISIBLE
                    textAlarm.setTextColor(itemView.context.getColor(R.color.error))
                }
                tower.overflowAlarm -> {
                    textAlarm.text = "⚠️ 溢水报警"
                    textAlarm.visibility = View.VISIBLE
                    textAlarm.setTextColor(itemView.context.getColor(R.color.warning))
                }
                else -> {
                    textAlarm.visibility = View.GONE
                }
            }
            
            // 水泵按钮
            if (tower.pumpOn) {
                btnPump.text = "停止水泵"
                btnPump.setBackgroundColor(itemView.context.getColor(R.color.error))
            } else {
                btnPump.text = "开启水泵"
                btnPump.setBackgroundColor(itemView.context.getColor(R.color.success))
            }
            
            // 最后更新时间
            val sdf = SimpleDateFormat("HH:mm:ss", Locale.CHINA)
            textUpdate.text = "更新：${sdf.format(Date(tower.lastUpdate))}"
            
            // 点击事件
            card.setOnClickListener { onTowerClick(tower) }
            btnPump.setOnClickListener { onPumpToggle(tower) }
        }
    }
    
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): TowerViewHolder {
        val view = LayoutInflater.from(parent.context)
            .inflate(R.layout.tower_item, parent, false)
        return TowerViewHolder(view)
    }
    
    override fun onBindViewHolder(holder: TowerViewHolder, position: Int) {
        holder.bind(towers[position])
    }
    
    override fun getItemCount() = towers.size
    
    fun updateData(newTowers: List<TowerData>) {
        towers = newTowers
        notifyDataSetChanged()
    }
}
