<?php
	
	require_once("php-pdb.inc");
	
	function saveStringWithLen(&$pdb, $str, $len) {
		while (strlen($str) < $len) {
			$str = $str . "\0";
		}
		
		$pdb->AppendString($str);
	}
	
	function readMbFile($filename) {
		// TODO: ���ɳ����������ĳɴ��ļ��ж�ȡ���
		
		$mb = array();
		
		// ���ݿ���
		$mb["dbName"] = "PIME_NM_MB";
		
		// �����Ϣ
		$mb["info"] = array();
		
		$mb["info"]["name"] = "�������뷨";
		$mb["info"]["shortName"] = "����";
		$mb["info"]["maxCodeLen"] = 4;
		$mb["info"]["smartCode"] = "z";
		$mb["info"]["codeCount"] = 16;
		$mb["info"]["codeChars"] = "0123456789abcdef";
		
		// ����¼
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
			
			$currentDbRecordIndex = (int)(2 + ($i / 1024)); // TODO: ��Ҫ���¼���Ӧ�����ĸ���¼�б��棬�Ա�ƽ�������ٶȡ�����Ч�ʺ����ݿ��¼����
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
		$mb = readMbFile(""); // TODO: �������ж�ȡ�ļ�������Ϊ�������롣
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