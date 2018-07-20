(**************************************************************************)
(*                                                                        *)
(*  This file is part of deliverable T3.3 of project INGOPCS              *)
(*                                                                        *)
(*    Copyright (C) 2017-2018 TrustInSoft                                 *)
(*                                                                        *)
(*  All rights reserved.                                                  *)
(*                                                                        *)
(**************************************************************************)

open Cil_types

let do_param kf stmt v =
  let lval = Cil.var v in
  let kinstr = Cil_types.Kstmt stmt in
    let loc =
      !Db.Value.lval_to_loc kinstr ~with_alarms:CilE.warn_none_mode lval
    in
    let state = Db.Value.get_stmt_state stmt in
    let value = Db.Value.find state loc in
    Format.printf "\t%a = %a@."
      Cil_datatype.Varinfo.pretty v Db.Value.pretty value

let process_kf kf =
  let fundec = Kernel_function.get_definition kf in
  let printer = new Printer.extensible_printer () in
  let pp_vdecl = printer#without_annot printer#vdecl in
  let vi_kf = fundec.svar in
  Format.printf "%a@." pp_vdecl vi_kf;
  let formals = Kernel_function.get_formals kf in
  let stmt = Kernel_function.find_first_stmt kf in
  List.iter (do_param kf stmt) formals

let main () =
  if not (Db.Value.is_computed ()) then
    failwith "ERROR: no value analysis results"
  else
  let do_function kf =
    if not (Tis_plugins.Tis_info.is_libc_kf kf)  then
      if !Db.Value.no_results kf then
        Format.printf "%a: no results available (skip)@."
          Kernel_function.pretty kf
      else if not (!Db.Value.is_called kf) then
        Format.printf "%a is unreachable (skip)@."
          Kernel_function.pretty kf
      else
        process_kf kf
  in Globals.Functions.iter do_function

let () = Db.Main.extend main
