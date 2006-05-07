<?php
	
	require_once("php-pdb.inc");
	
	function saveStringWithLen(&$pdb, $str, $len) {
		while (strlen($str) < $len) {
			$str = $str . "\0";
		}
		
		$pdb->AppendString($str);
	}
	
	function readMbFile($filename) {
		// TODO: 将由程序生成码表改成从文件中读取码表。
		
		$mb = array();
		
		// 数据库名
		$mb["dbName"] = "PIME_NM_MB";
		
		// 码表信息
		$mb["info"] = array();
		
		$mb["info"]["name"] = "内码输入法";
		$mb["info"]["shortName"] = "内码";
		$mb["info"]["maxCodeLen"] = 4;
		$mb["info"]["smartCode"] = "z";
		$mb["info"]["codeCount"] = 16;
		$mb["info"]["codeChars"] = "0123456789abcdef";
		
		// 码表记录
		$mb["records"] = array();
		
		for ($i = 0x81; $i < 0xFF; $i ++) {
			for ($j = 0x40; $j < 0xFF; $j ++) {
				if ($j == 0x7F) continue;
				
				$code = sprintf("%x", ($i << 8) + $j);
				$word = chr($i) . chr($j);
				
				$record = array();
				$record["code"] = $code;
				$record["word"] = $word . "\0\0";
				
				array_push($mb["records"], $record);
			}
		}
		
		return $mb;
	}
	
	function createPdb($mb) {
		$pdb = new PalmDB("IMMB", "PIme", $mb["dbName"]);
		
		return $pdb;
	}
	
	function saveMbInfo(&$pdb, $mb) {
		$pdb->GoToRecord(0);
		
		saveStringWithLen($pdb, $mb["info"]["name"], 0x10);
		saveStringWithLen($pdb, $mb["info"]["shortName"], 0x05);
		$pdb->AppendInt8($mb["info"]["maxCodeLen"]);
		$pdb->AppendString($mb["info"]["smartCode"], 1);
		$pdb->AppendInt8($mb["info"]["codeCount"]);
		saveStringWithLen($pdb, $mb["info"]["codeChars"], $mb["info"]["codeCount"]);
	}
	
	function saveMbIndex(&$pdb, $mb) {
		$pdb->GoToRecord(1);
		
		$pdb->AppendInt16(0);
	}
	
	function saveMbRecords(&$pdb, $mb) {
		$wordOffset = 0;
		
		for ($i = 0; $i < count($mb["records"]); $i ++) {
			$code = $mb["records"][$i]["code"];
			$word = $mb["records"][$i]["word"];
			
			$currentDbRecordIndex = (int)(2 + ($i / 1024)); // TODO: 需要重新计算应该在哪个记录中保存，以便平衡搜索速度、索引效率和数据库记录数。
			$pdb->GoToRecord($currentDbRecordIndex);
			
			$codeCount = 1024;
			$maxCodeLen = $mb["info"]["maxCodeLen"];
			
			if (!$pdb->RecordExists())
			{
				$wordOffset = 2 + $codeCount * ($maxCodeLen + 2);
				
				$pdb->AppendInt16($codeCount);
				
				$words = array();
			}
			
			saveStringWithLen($pdb, $code, $maxCodeLen);
			$pdb->AppendInt16($wordOffset);
			
			$wordOffset += strlen($word);
			
			array_push($words, $word);
			
			$nextDbRecordIndex = (int)(2 + (($i + 1) / 1024));
			if ($nextDbRecordIndex != $currentDbRecordIndex) {
				foreach ($words as $w) {
					saveStringWithLen($pdb, $w, strlen($w));
				}
			}
		}
		
		$pdb->AppendInt16(0x1234);
	}
	
	function savePdb($pdb, $mb) {
		$fp = fopen($mb["dbName"] . ".pdb", "wb");
		
		if (!$fp) return false;
		
		$pdb->WriteToFile($fp);
		fclose($fp);
		
		return true;
	}
	
	function main() {
		$mb = readMbFile(""); // TODO: 从命令行读取文件名，作为参数传入。
		if ($mb == null) {
			echo "Could not read the mb file!\n";
			return false;
		}
		
		$pdb = createPdb($mb);
		
		saveMbInfo($pdb, $mb);
		
		saveMbRecords($pdb, $mb);
		
		saveMbIndex($pdb, $mb);
		
		if (!savePdb($pdb, $mb)) {
			echo "Could not save PDB file!\n";
			return false;
		}
		else
			echo "PDB File saved.\n";
		
		return true;
	}
	
	main();
?>