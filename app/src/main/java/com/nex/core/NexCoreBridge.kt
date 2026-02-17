package com.nex.core

object NexCoreBridge {
    init { System.loadLibrary("nexcore") }

    external fun propose(userText: String, baseDir: String): String
    external fun execute(proposalId: String, approved: Boolean, baseDir: String): String
    external fun auditTail(maxEntries: Int, baseDir: String): String
}
