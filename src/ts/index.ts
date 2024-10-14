import fs from "fs"
import path from "path"

const rootDir = path.join(__dirname, "..", "..")
const oopetris = require("node-gyp-build")(rootDir)

export type U64 = number | BigInt
export type U32 = number
export type U8 = number

export type AdditionalInformationValue =
	| string
	| U8
	| U32
	| U64
	| boolean
	| AdditionalInformationValue[]

export type AdditionalInformation = Record<string, AdditionalInformationValue>

export type InputEvent =
	| "RotateLeftPressed"
	| "RotateRightPressed"
	| "MoveLeftPressed"
	| "MoveRightPressed"
	| "MoveDownPressed"
	| "DropPressed"
	| "HoldPressed"
	| "RotateLeftReleased"
	| "RotateRightReleased"
	| "MoveLeftReleased"
	| "MoveRightReleased"
	| "MoveDownReleased"
	| "DropReleased"
	| "HoldReleased"

export type TetrionRecord = {
	event: InputEvent
	simulation_step_index: U64
	tetrion_index: U8
}

export type MinoPosition = {
	x: U8
	y: U8
}

export type TetrominoType = "I" | "J" | "L" | "O" | "S" | "T" | "Z"

export type Mino = {
	position: MinoPosition
	type: TetrominoType
}

export type TetrionSnapshot = {
	level: U32
	lines_cleared: U32
	mino_stack: Mino[]
	score: U64
	simulation_step_index: U64
	tetrion_index: U8
}

export type TetrionHeader = {
	seed: U64
	starting_level: U32
}

export type RecordingInformation = {
	information: AdditionalInformation
	records: TetrionRecord[]
	snapshots: TetrionSnapshot[]
	tetrion_headers: TetrionHeader[]
	version: U8
}

export type GridProperties = {
	height: U8
	width: U8
}

export type RecordingsProperties = {
	gridProperties: GridProperties
}

export class RecordingsUtility {
	static isRecordingFile(file: string): boolean {
		try {
			// this throws, when file is not an string or not there at all, just be safe for JS land
			return oopetris.isRecordingFile(file)
		} catch (_err) {
			return false
		}
	}

	static getInformation(file: string): null | RecordingInformation {
		if (!fs.existsSync(file)) {
			return null
		}

		try {
			if (!RecordingsUtility.isRecordingFile(file)) {
				return null
			}

			// this throws, when file is not an string, not there at all, or some other error regarding serialization from c++ land to JS land occurs,  just be safe for JS land
			return oopetris.getInformation(file)
		} catch (_err) {
			return null
		}
	}

	static get properties(): GridProperties {
		return oopetris.properties
	}

	static get version(): string {
		return oopetris.version
	}
}
