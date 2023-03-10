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
										if (command_ == 2)
										{
											///////// code /////////
										}
										else // command_ == 3
										{
											///////// code /////////
										}
									}
								}
								else // command_ >= 4
								{
									if (command_ < 6)
									{
										if (command_ == 4)
										{
											///////// code /////////
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

			}
		}
		else // command_ >= 256
		{

		}

		switch (command_)
		{
		case SendObj:
			memory_.push_back(data_->front());
			data_->erase(data_->begin());
		case BringObj:
			break;
				
		case SumNumbers:
			AlifObject* left = memory_.front();
			memory_.erase(memory_.begin());
			AlifObject* right = memory_.front();
			memory_.erase(memory_.begin());

			AlifObject* result = new AlifObject();
			result->objType = OTNumber;
			left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? result->V.NumberObj.numberType = TTFloat : result->V.NumberObj.numberType = TTInteger;
			result->V.NumberObj.numberValue = left->V.NumberObj.numberValue + right->V.NumberObj.numberValue;
			memory_.push_back(result);
			break;
		//default:
		//	std::wcout << memory_.front()->V.NumberObj.numberValue << std::endl;
		//	break;
		}
	}
}