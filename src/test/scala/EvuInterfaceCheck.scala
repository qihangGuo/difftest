package difftest

import chisel3._

/** Generate the real C++ ABI without elaborating a full CPU or emitting RTL. */
object EvuInterfaceCheck {
  def main(args: Array[String]): Unit = {
    circt.stage.ChiselStage.emitCHIRRTL(new Module {
      val common = Seq(
        new DiffArchEvent, new DiffInstrCommit(192), new DiffTrapEvent,
        new DiffCSRState, new DiffHCSRState, new DiffVecCSRState,
        new DiffFpCSRState, new DiffArchIntRegState, new DiffArchFpRegState,
        new DiffArchVecRegState, new DiffEvuStoreEvent, new DiffEvuCompletionEvent,
        new DiffEvuCSRState
      )
      common.foreach(bundle => DifftestModule(bundle, dontCare = true))
      for (_ <- 0 until 32) DifftestModule(new DiffEvuVrfState(64), dontCare = true)
      DifftestModule.collect("XiangShan")
    })
    println("EVU C++ interface generated")
  }
}
