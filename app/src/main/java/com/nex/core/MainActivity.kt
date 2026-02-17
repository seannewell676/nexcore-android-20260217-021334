package com.nex.core

import android.os.Bundle
import android.speech.tts.TextToSpeech
import androidx.appcompat.app.AppCompatActivity
import com.nex.core.databinding.ActivityMainBinding
import java.io.File
import java.util.Locale

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private var tts: TextToSpeech? = null

    private var lastProposalId: String? = null
    private var lastSpoken: String = ""

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val baseDir = File(filesDir, "nex_runtime").absolutePath
        File(baseDir).mkdirs()

        tts = TextToSpeech(this) { status ->
            if (status == TextToSpeech.SUCCESS) {
                tts?.language = Locale.US
            }
        }

        binding.btnPropose.setOnClickListener {
            val input = binding.inputText.text?.toString().orEmpty()
            if (input.isBlank()) return@setOnClickListener
            val resultJson = NexCoreBridge.propose(input, baseDir)
            binding.outputText.text = resultJson
            lastSpoken = resultJson

            lastProposalId = Regex("\\.proposal_id\\.\\.s*:\\.s*\\.([^\\.]+)\\.")
                .find(resultJson)?.groupValues?.get(1)
        }

        binding.btnExecute.setOnClickListener {
            val pid = lastProposalId
            if (pid.isNullOrBlank()) {
                binding.outputText.text = "No proposal_id yet. Tap Propose first."
                lastSpoken = binding.outputText.text.toString()
                return@setOnClickListener
            }
            val receiptJson = NexCoreBridge.execute(pid, true, baseDir)
            binding.outputText.text = receiptJson
            lastSpoken = receiptJson
        }

        binding.btnSpeak.setOnClickListener {
            speak(lastSpoken.ifBlank { binding.outputText.text?.toString().orEmpty() })
        }
    }

    private fun speak(text: String) {
        if (text.isBlank()) return
        tts?.speak(text, TextToSpeech.QUEUE_FLUSH, null, "NEX_TTS")
    }

    override fun onDestroy() {
        tts?.stop()
        tts?.shutdown()
        super.onDestroy()
    }
}
