#include "Interpreter.h"

Interpreter::Interpreter(std::vector<InstructionsType>* _instructions, std::vector<AlifObject*>* _data) :
	instructions_(_instructions), data_(_data) {}

void Interpreter::run_code()
{
	for (InstructionsType command_ : *instructions_)
	{
		if (command_ < 256) 
		{
			if (command_ < 128)
			{
				if (command_ < 64)
				{
					if (command_ < 32)
					{
						if (command_ < 16)
						{
							if (command_ < 8)
							{
								if (command_ < 4)
								{
									if (command_ < 2)
									{
										if (command_ == 0)
										{
											///////// code /////////
										}
										else // command_ == 1
										{
											///////// code /////////
										}
									}
									else // command_ >= 2
									{
										if (command_ == 2) // SEND_MEM
										{
											*(memory_ + stackLevel) = data_->front();
											stackLevel--;
											data_->erase(data_->begin());
										}
										else // command_ == 3 --> ADD_OP
										{
											stackLevel++;
											AlifObject* left = *(memory_ + stackLevel);
											stackLevel++;
											AlifObject* right = *(memory_ + stackLevel);

											AlifObject* result = new AlifObject();
											result->objType = OTNumber;
											left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? result->V.NumberObj.numberType = TTFloat : result->V.NumberObj.numberType = TTInteger;
											result->V.NumberObj.numberValue = right->V.NumberObj.numberValue + left->V.NumberObj.numberValue;
											*(memory_ + stackLevel) = result;
											stackLevel--;
										}
									}
								}
								else // command_ >= 4
								{
									if (command_ < 6)
									{
										if (command_ == 4) // MINUS_OP
										{
											stackLevel++;
											AlifObject* left = *(memory_ + stackLevel);
											stackLevel++;
											AlifObject* right = *(memory_ + stackLevel);

											AlifObject* result = new AlifObject();
											result->objType = OTNumber;
											left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? result->V.NumberObj.numberType = TTFloat : result->V.NumberObj.numberType = TTInteger;
											result->V.NumberObj.numberValue = right->V.NumberObj.numberValue - left->V.NumberObj.numberValue;
											*(memory_ + stackLevel) = result;
											stackLevel--;
										}
										else // command_ == 5
										{
											///////// code /////////
										}
									}
									else // command_ >= 6
									{
										if (command_ == 6)
										{

										}
										else // command_ == 7
										{
											
										}
									}
								}
							}
							else // command_ >= 8
							{
								if (command_ < 12)
								{
									if (command_ < 10)
									{
										if (command_ == 8)
										{

										}
										else // command_ == 9
										{

										}
									}
									else // command_ >= 10
									{
										if (command_ == 10)
										{
											
										}
										else // command_ == 11
										{

										}
									}
								}
								else // command >= 12
								{
									if (command_ < 14)
									{
										if (command_ == 12)
										{

										}
										else // command_ == 13
										{

										}
									}
									else // command_ >= 14
									{
										if (command_ == 14)
										{

										}
										else // command_ == 15
										{

										}
									}
								}
							}
						}
						else // command_ >= 16
						{
							if (command_ < 24)
							{
								if (command_ < 20)
								{
									if (command_ < 18)
									{
										if (command_ == 16)
										{

										}
										else // command_ == 17
										{

										}
									}
									else // command_ >= 18
									{
										if (command_ == 18)
										{

										}
										else // command_ == 19
										{

										}
									}
								}
								else // command_ >= 20
								{
									if (command_ < 22)
									{
										if (command_ == 20)
										{

										}
										else // command_ == 21
										{

										}
									}
									else // command_ >= 22
									{
										if (command_ == 22)
										{

										}
										else // command_ == 23
										{

										}
									}
								}
							}
							else // command_ >= 24
							{
								if (command_ < 28)
								{
									if (command_ < 26)
									{
										if (command_ == 24)
										{

										}
										else // command_ == 25
										{

										}
									}
									else // command_ >= 26
									{
										if (command_ == 26)
										{

										}
										else // command_ == 27
										{

										}
									}
								}
								else // command_ >= 28
								{
									if (command_ < 30)
									{
										if (command_ == 28)
										{

										}
										else // command_ == 29
										{

										}
									}
									else // command_ >= 30
									{
										if (command_ == 30)
										{

										}
										else // command == 31
										{

										}
									}
								}
							}
						}
					}
					else // command_ >= 32
					{
						if (command_ < 48)
						{
							if (command_ < 40)
							{
								if (command_ < 36)
								{
									if (command_ < 34)
									{
										if (command_ == 32)
										{

										}
										else // command_ == 33
										{

										}
									}
									else // command_ >= 34
									{
										if (command_ == 34)
										{

										}
										else // command_ == 35
										{

										}
									}
								}
								else // command_ >= 36
								{
									if (command_ < 38)
									{
										if (command_ == 36)
										{

										}
										else // command_ == 37
										{

										}
									}
									else // command_ >= 38
									{
										if (command_ == 38)
										{

										}
										else // command_ == 39
										{

										}
									}
								}
							}
							else // command_ >= 40
							{
								if (command_ < 44)
								{
									if (command_ < 42)
									{
										if (command_ == 40)
										{

										}
										else // command == 41
										{

										}
									}
									else // command_ >= 42
									{
										if (command_ == 42)
										{

										}
										else // command_ == 43
										{

										}
									}
								}
								else // command_ >= 44
								{
									if (command_ < 46)
									{
										if (command_ == 44)
										{

										}
										else // command_ == 45
										{

										}
									}
									else // command_ >= 46
									{
										if (command_ == 46)
										{

										}
										else // command_ == 47
										{

										}
									}
								}
							}
						}
						else // command >= 48
						{
							if (command_ < 56)
							{
								if (command_ < 52)
								{
									if (command_ < 50)
									{
										if (command_ == 48)
										{

										}
										else // command_ == 49
										{

										}
									}
									else // command_ >= 50
									{
										if (command_ == 50)
										{

										}
										else // command_ == 51
										{

										}
									}
								}
								else // command_ >= 52
								{
									if (command_ < 54)
									{
										if (command_ == 52)
										{

										}
										else // command_ == 53
										{

										}
									}
									else // command_ >= 54
									{
										if (command_ == 54)
										{

										}
										else // command_ == 55
										{

										}
									}
								}
							}
							else // command_ >= 56
							{
								if (command_ < 60)
								{
									if (command_ < 58)
									{
										if (command_ == 56)
										{

										}
										else // command_ == 57
										{

										}
									}
									else // command_ >= 58
									{
										if (command_ == 58)
										{

										}
										else // command_ == 59
										{

										}
									}
								}
								else // command_ >= 60
								{
									if (command_ < 62)
									{
										if (command_ == 60)
										{

										}
										else // command_ == 61
										{

										}
									}
									else // command_ >= 62
									{
										if (command_ == 62)
										{

										}
										else // command == 63
										{

										}
									}
								}
							}
						}
					}
				}
				else // command_ >= 64
				{
					if (command_ < 96)
					{
						if (command_ < 80)
						{
							if (command_ < 72)
							{
								if (command_ < 68)
								{
									if (command_ < 66)
									{
										if (command_ == 64)
										{

										}
										else // command_ == 65
										{

										}
									}
									else // command_ >= 66
									{
										if (command_ == 66)
										{

										}
										else // command_ == 67
										{

										}
									}
								}
								else // command_ >= 68
								{
									if (command_ < 70)
									{
										if (command_ == 68)
										{

										}
										else // command_ == 69
										{

										}
									}
									else // command_ >= 70
									{
										if (command_ == 70)
										{

										}
										else // command_ == 71
										{

										}
									}
								}
							}
							else // command_ >= 72
							{
								if (command_ < 76)
								{
									if (command_ < 74)
									{
										if (command_ == 72)
										{

										}
										else // command_ == 73
										{

										}
									}
									else // command_ >= 74
									{
										if (command_ == 74)
										{

										}
										else // command_ == 75
										{

										}
									}
								}
								else // command_ >= 76
								{
									if (command_ < 78)
									{
										if (command_ == 76)
										{

										}
										else // command_ == 77
										{

										}
									}
									else // command_ >= 78
									{
										if (command_ == 78)
										{

										}
										else // command_ == 79
										{

										}
									}
								}
							}
						}
						else // command_ >= 80
						{
							if (command_ < 88)
							{
								if (command_ < 84)
								{
									if (command_ < 82)
									{
										if (command_ == 80)
										{

										}
										else // command_ == 81
										{

										}
									}
									else // command_ >= 82
									{
										if (command_ == 82)
										{

										}
										else // command_ == 83
										{

										}
									}
								}
								else // command_ >= 84
								{
									if (command_ < 86)
									{
										if (command_ == 84)
										{

										}
										else // command_ == 85
										{

										}
									}
									else // command_ >= 86
									{
										if (command_ == 86)
										{

										}
										else // command_ == 87
										{

										}
									}
								}
							}
							else // command_ >= 88
							{
								if (command_ < 92)
								{
									if (command_ < 90)
									{
										if (command_ == 88)
										{

										}
										else // command_ == 89
										{

										}
									}
									else // command_ >= 90
									{
										if (command_ == 90)
										{

										}
										else // command_ == 91
										{

										}
									}
								}
								else // command_ >= 92
								{
									if (command_ < 94)
									{
										if (command_ == 92)
										{

										}
										else // command_ == 93
										{

										}
									}
									else // command_ >= 94
									{
										if (command_ == 94)
										{

										}
										else // command_ == 95
										{

										}
									}
								}
							}
						}
					}
					else // command_ >= 96
					{
						if (command_ < 112)
						{
							if (command_ < 104)
							{
								if (command_ < 100)
								{
									if (command_ < 98)
									{
										if (command_ == 96)
										{

										}
										else // command_ == 97
										{

										}
									}
									else // command_ >= 98
									{
										if (command_ == 98)
										{

										}
										else // command_ == 99
										{

										}
									}
								}
								else // command >= 100
								{
									if (command_ < 102)
									{
										if (command_ == 100)
										{

										}
										else // command_ == 101
										{

										}
									}
									else // command_ >= 102
									{
										if (command_ == 102)
										{

										}
										else // command_ == 103
										{

										}
									}
								}
							}
							else // command_ >= 104
							{
								if (command_ < 108)
								{
									if (command_ < 106)
									{
										if (command_ == 104)
										{

										}
										else // command_ == 105
										{

										}
									}
									else // command_ >= 106
									{
										if (command_ == 106)
										{

										}
										else // command_ == 107
										{

										}
									}
								}
								else // command_ >= 108
								{
									if (command_ < 110)
									{
										if (command_ == 108)
										{

										}
										else // command_ == 109
										{

										}
									}
									else // command_ >= 110
									{
										if (command_ == 110)
										{

										}
										else // command_ == 111
										{

										}
									}
								}
							}
						}
						else // command_ >= 112
						{
							if (command_ < 120)
							{
								if (command_ < 116)
								{
									if (command_ < 114)
									{
										if (command_ == 112)
										{

										}
										else // command_ == 113
										{

										}
									}
									else // command_ >= 114
									{
										if (command_ == 114)
										{

										}
										else // command_ == 115
										{

										}
									}
								}
								else // command_ >= 116
								{
									if (command_ < 118)
									{
										if (command_ == 116)
										{

										}
										else // command_ == 117
										{

										}
									}
									else // command_ >= 118
									{
										if (command_ == 118)
										{

										}
										else // command_ == 119
										{

										}
									}
								}
							}
							else // command_ >= 120
							{
								if (command_ < 124)
								{
									if (command_ < 122)
									{
										if (command_ == 120)
										{

										}
										else // command_ == 121
										{

										}
									}
									else // command_ >= 122
									{
										if (command_ == 122)
										{

										}
										else // command_ == 123
										{

										}
									}
								}
								else // command_ >= 124
								{
									if (command_ < 126)
									{
										if (command_ == 124)
										{

										}
										else // command_ == 125
										{

										}
									}
									else // command_ >= 126
									{
										if (command_ == 126)
										{

										}
										else // command_ == 127
										{

										}
									}
								}
							}
						}
					}
				}
			}
			else // command_ >= 128
			{
				if (command_ < 192)
				{
					if (command_ < 160)
					{
						if (command_ < 144)
						{
							if (command_ < 136)
							{
								if (command_ < 132)
								{
									if (command_ < 130)
									{
										if (command_ == 128)
										{

										}
										else // command_ == 129
										{

										}
									}
									else // command_ >= 130
									{
										if (command_ == 130)
										{

										}
										else // command_ == 131
										{

										}
									}
								}
								else // command_ >= 132
								{
									if (command_ < 134)
									{
										if (command_ == 132)
										{

										}
										else // command_ == 133
										{

										}
									}
									else // command_ >= 134
									{
										if (command_ == 134)
										{

										}
										else // command_ == 135
										{

										}
									}
								}
							}
							else // command_ >= 136
							{
								if (command_ < 140)
								{
									if (command_ < 138)
									{
										if (command_ == 136)
										{

										}
										else // command_ == 137
										{

										}
									}
									else // command_ >= 138
									{
										if (command_ == 138)
										{

										}
										else // command_ == 139
										{

										}
									}
								}
								else // command_ >= 140
								{
									if (command_ < 142)
									{
										if (command_ == 140)
										{

										}
										else // command_ == 141
										{

										}
									}
									else // command_ >= 142
									{
										if (command_ == 142)
										{

										}
										else // command_ == 143
										{

										}
									}
								}
							}
						}
						else // command_ >= 144
						{
							if (command_ < 152)
							{
								if (command_ < 148)
								{
									if (command_ < 146)
									{
										if (command_ == 144)
										{

										}
										else // command_ == 145
										{

										}
									}
									else // command_ >= 146
									{
										if (command_ == 146)
										{

										}
										else // command_ == 147
										{

										}
									}
								}
								else // command_ >= 148
								{
									if (command_ < 150)
									{
										if (command_ == 148)
										{

										}
										else // ocmmand_ == 149
										{

										}
									}
									else // command_ >= 150
									{
										if (command_ == 150)
										{

										}
										else // command_ == 151
										{

										}
									}
								}
							}
							else // command_ >= 152
							{
								if (command_ < 156)
								{

									if (command_ < 154)
									{
										if (command_ == 152)
										{

										}
										else // command_ == 153
										{

										}
									}
									else // command_ >= 154
									{
										if (command_ == 154)
										{

										}
										else // command_ == 155
										{

										}
									}
								}
								else // command_ >= 156
								{
									if (command_ < 158)
									{
										if (command_ == 156)
										{

										}
										else // command_ == 157
										{

										}
									}
									else // command_ >= 158
									{
										if (command_ == 158)
										{

										}
										else // command_ == 159
										{

										}
									}
								}
							}
						}
					}
					else // command_ >= 160
					{
						if (command_ < 176)
						{
							if (command_ < 168)
							{
								if (command_ < 164)
								{
									if (command_ < 162)
									{
										if (command_ == 160)
										{

										}
										else // command_ == 161
										{

										}
									}
									else // command_ >= 162
									{
										if (command_ == 162)
										{

										}
										else // command_ == 163
										{

										}
									}
								}
								else // command_ >= 164
								{
									if (command_ < 166)
									{
										if (command_ == 164)
										{

										}
										else // command_ == 165
										{

										}
									}
									else // command_ >= 166
									{
										if (command_ == 166)
										{

										}
										else // command_ == 167
										{

										}
									}
								}
							}
							else // command_ >= 168
							{
								if (command_ < 172)
								{
									if (command_ < 170)
									{
										if (command_ == 168)
										{

										}
										else // command_ == 169
										{

										}
									}
									else // command_ >= 170
									{
										if (command_ == 170)
										{

										}
										else // command_ == 171
										{

										}
									}
								}
								else // command_ >= 172
								{
									if (command_ < 174)
									{
										if (command_ == 172)
										{

										}
										else // command_ == 173
										{

										}
									}
									else // command_ >= 174
									{
										if (command_ == 174)
										{

										}
										else // command_ == 175
										{

										}
									}
								}
							}
						}
						else // command_ >= 176
						{
							if (command_ < 184)
							{
								if (command_ < 180)
								{
									if (command_ < 178)
									{
										if (command_ == 176)
										{

										}
										else // command_ == 177
										{

										}
									}
									else // command_ >= 178
									{
										if (command_ == 178)
										{

										}
										else // command_ == 179
										{

										}
									}
								}
								else // command_ >= 180
								{
									if (command_ < 182)
									{
										if (command_ == 180)
										{

										}
										else // command_ == 181
										{

										}
									}
									else // command_ >= 182
									{
										if (command_ == 182)
										{

										}
										else // command_ == 183
										{

										}
									}
								}
							}
							else // command_ >= 184
							{
								if (command_ < 188)
								{
									if (command_ < 186)
									{
										if (command_ == 184)
										{

										}
										else // command_ == 185
										{

										}
									}
									else // command_ >= 186
									{
										if (command_ == 186)
										{

										}
										else // command_ == 187
										{

										}
									}
								}
								else // command_ >= 188
								{
									if (command_ < 190)
									{
										if (command_ == 188)
										{

										}
										else // command_ == 189
										{

										}
									}
									else // command_ >= 190
									{
										if (command_ == 190)
										{

										}
										else // command_ == 191
										{

										}
									}
								}
							}
						}
					}
				}
				else // command_ >= 192
				{
					if (command_ < 224)
					{
						if (command_ < 208)
						{
							if (command_ < 200)
							{
								if (command_ < 196)
								{
									if (command_ < 194)
									{
										if (command_ == 192)
										{

										}
										else // command_ == 193
										{

										}
									}
									else // command_ >= 194
									{
										if (command_ == 194)
										{

										}
										else // command_ == 195
										{

										}
									}
								}
								else // command_ >= 196
								{
									if (command_ < 198)
									{
										if (command_ == 196)
										{

										}
										else // command_ == 197
										{

										}
									}
									else // command_ >= 198
									{
										if (command_ == 198)
										{

										}
										else // command_ == 199
										{

										}
									}
								}
							}
							else // command_ >= 200
							{
								if (command_ < 204)
								{
									if (command_ < 202)
									{
										if (command_ == 200)
										{

										}
										else // command_ == 201
										{

										}
									}
									else // command_ >= 202
									{
										if (command_ == 202)
										{

										}
										else // command_ == 203
										{

										}
									}
								}
								else // command_ >= 204
								{
									if (command_ < 206)
									{
										if (command_ == 204)
										{

										}
										else // command_ == 205
										{

										}
									}
									else // command_ >= 206
									{
										if (command_ == 206)
										{

										}
										else // command_ == 207
										{

										}
									}
								}
							}
						}
						else // command_ >= 208
						{
							if (command_ < 216)
							{
								if (command_ < 212)
								{
									if (command_ < 210)
									{
										if (command_ == 208)
										{

										}
										else // command_ == 209
										{

										}
									}
									else // command_ >= 210
									{
										if (command_ == 210)
										{

										}
										else // command_ == 211
										{

										}
									}
								}
								else // command_ >= 212
								{
									if (command_ < 214)
									{
										if (command_ == 212)
										{

										}
										else // command_ == 213
										{

										}
									}
									else // command_ >= 214
									{
										if (command_ == 214)
										{

										}
										else // command_ == 215
										{

										}
									}
								}
							}
							else // command_ >= 216
							{
								if (command_ < 220)
								{
									if (command_ < 218)
									{
										if (command_ == 216)
										{

										}
										else // command_ == 217
										{

										}
									}
									else // command_ >= 218
									{
										if (command_ == 218)
										{

										}
										else // command_ == 219
										{

										}
									}
								}
								else // command_ >= 220
								{
									if (command_ < 222)
									{
										if (command_ == 220)
										{

										}
										else // command_ == 221
										{

										}
									}
									else // command_ >= 222
									{
										if (command_ == 222)
										{

										}
										else // command_ == 223
										{

										}
									}
								}
							}
						}
					}
					else // command_ >= 224
					{
						if (command_ < 240)
						{
							if (command_ < 232)
							{
								if (command_ < 228)
								{
									if (command_ < 226)
									{
										if (command_ == 224)
										{

										}
										else // command_ == 225
										{

										}
									}
									else // command_ >= 226
									{
										if (command_ == 226)
										{

										}
										else// command_ == 227
										{

										}
									}
								}
								else // command_ >= 228
								{
									if (command_ < 230)
									{
										if (command_ == 228)
										{

										}
										else // command_ == 229
										{

										}
									}
									else // command_ >= 230
									{
										if (command_ == 230)
										{

										}
										else // command_ == 231
										{

										}
									}
								}
							}
							else // command_ >= 232
							{
								if (command_ < 236)
								{
									if (command_ < 234)
									{
										if (command_ == 232)
										{

										}
										else // command_ == 233
										{

										}
									}
									else // command_ >= 234
									{
										if (command_ == 234)
										{

										}
										else // command_ == 235
										{

										}
									}
								}
								else // command_ >= 236
								{
									if (command_ < 238)
									{
										if (command_ == 236)
										{

										}
										else // command_ == 237
										{

										}
									}
									else // command_ >= 238
									{
										if (command_ == 238)
										{

										}
										else // command_ == 239
										{

										}
									}
								}
							}
						}
						else // command_ >= 240
						{
							if (command_ < 248)
							{
								if (command_ < 244)
								{
									if (command_ < 242)
									{
										if (command_ == 240)
										{

										}
										else // command_ == 241
										{

										}
									}
									else // command_ >= 242
									{
										if (command_ == 242)
										{

										}
										else // command_ == 243
										{

										}
									}
								}
								else // command_ >= 244
								{
									if (command_ < 246)
									{
										if (command_ == 244)
										{

										}
										else // command_ == 245
										{

										}
									}
									else // command_ >= 246
									{
										if (command_ == 246)
										{

										}
										else // command_ == 247
										{

										}
									}
								}
							}
							else // command_ >= 248
							{
								if (command_ < 252)
								{
									if (command_ < 250)
									{
										if (command_ == 248)
										{

										}
										else // command_ == 249
										{

										}
									}
									else // command_ >= 250
									{
										if (command_ == 250)
										{

										}
										else // command_ 251
										{

										}
									}
								}
								else // command_ >= 252
								{
									if (command_ < 254)
									{
										if (command_ == 252)
										{

										}
										else // command_ == 253
										{

										}
									}
									else // command_ >= 254
									{
										if (command_ == 254)
										{

										}
										else // command_ == 255
										{

										}
									}
								}
							}
						}
					}
				}
			}
		}
		else // command_ >= 256
		{

		}
	}

	stackLevel++;
	AlifObject* res = *(memory_ + stackLevel);
	std::wcout << res->V.NumberObj.numberValue << std::endl;
}
