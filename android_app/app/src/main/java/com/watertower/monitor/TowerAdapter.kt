package com.watertower.monitor

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.watertower.monitor.databinding.ItemTowerBinding
import java.text.SimpleDateFormat
import java.util.*

class TowerAdapter : ListAdapter<TowerData, TowerAdapter.TowerViewHolder>(TowerDiffCallback()) {
    
    var onPumpClick: ((TowerData) -> Unit)? = null
    
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): TowerViewHolder {
        val binding = ItemTowerBinding.inflate(
            LayoutInflater.from(parent.context),
            parent,
            false
        )
        return TowerViewHolder(binding)
    }
    
    override fun onBindViewHolder(holder: TowerViewHolder, position: Int) {
        holder.bind(getItem(position))
    }
    
    inner class TowerViewHolder(
        private val binding: ItemTowerBinding
    ) : RecyclerView.ViewHolder(binding.root) {
        
        fun bind(tower: TowerData) {
            binding.textTowerName.text = "${tower.name} (${tower.id})"
            binding.textWaterLevel.text = "${tower.waterLevel}%"
            binding.progressWater.progress = tower.waterLevel
            
            // 在线状态
            if (tower.online) {
                binding.textStatus.text = "在线"
                binding textStatus.setTextColor(
                    binding.root.context.getColor(R.color.online_green)
                )
            } else {
                binding.textStatus.text = "离线"
                binding.textStatus.setTextColor(
                    binding.root.context.getColor(R.color.offline_red)
                )
            }
            
            // 水泵状态
            if (tower.pumpOn) {
                binding.btnPump.text = "停止水泵"
                binding.btnPump.setBackgroundColor(
                    binding.root.context.getColor(R.color.pump_on_red)
                )
            } else {
                binding.btnPump.text = "开启水泵"
                binding.btnPump.setBackgroundColor(
                    binding.root.context.getColor(R.color.pump_off_green)
                )
            }
            
            // 报警状态
            if (tower.lowWaterAlarm) {
                binding.textAlarm.text = "⚠️ 缺水"
                binding.textAlarm.visibility = android.view.View.VISIBLE
            } else if (tower.overflowAlarm) {
                binding.textAlarm.text = "⚠️ 溢水"
                binding.textAlarm.visibility = android.view.View.VISIBLE
            } else {
                binding.textAlarm.visibility = android.view.View.GONE
            }
            
            // 最后更新时间
            val sdf = SimpleDateFormat("HH:mm:ss", Locale.CHINA)
            binding.textLastUpdate.text = "更新：${sdf.format(Date(tower.lastUpdate))}"
            
            // 点击事件
            binding.btnPump.setOnClickListener {
                onPumpClick?.invoke(tower)
            }
        }
    }
    
    class TowerDiffCallback : DiffUtil.ItemCallback<TowerData>() {
        override fun areItemsTheSame(oldItem: TowerData, newItem: TowerData): Boolean {
            return oldItem.id == newItem.id
        }
        
        override fun areContentsTheSame(oldItem: TowerData, newItem: TowerData): Boolean {
            return oldItem == newItem
        }
    }
}
