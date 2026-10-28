from pathlib import Path
import json,copy,sys
ROOT=Path(__file__).parent
GAME=Path(r'C:\Program Files (x86)\Steam\steamapps\common\Space Rangers HD A War Apart')
sys.path.insert(0,str(GAME))
from srhd_modkit.blockpar import parse_blockpar
base=json.loads(Path(r'S:\SR\DesignDepartment.rson').read_text(encoding='utf-8-sig'))
template=base['Visual.Objects'][0]
group={k:[] for k,v in template.items() if isinstance(v,list)}
group['ConstellationCount']=0
seq=-1
messages=[]
def obj(category,**fields):
 global seq
 is_message=category=='DialogMessages'
 if is_message:
  category='Operations'
  fields.pop('DMsg.Num',None)
  fields['Code']=fields.pop('Msg').splitlines()
  fields['Code.Type']='Turn'
 original=template[category][0]
 value=copy.deepcopy(original);seq+=1
 value.update({'#':seq,'Parent':-1,'Pos.x':0,'Pos.y':seq*30});value.update(fields)
 group[category].append(value)
 if is_message:messages.append(value)
 return seq
original=[
('Как работа?','Кто-то открывает новые миры. Мы убираем то, что они оставили по дороге.'),
('Что сегодня нашли?','Болт, пустую цистерну и чью-то великую мечту. Мечту оставили на месте.'),
('Зачем убирать такую мелочь?','На орбите мелочь - это вопрос скорости.'),
('Работа не надоедает?','Иногда. Потом смотришь на звёзды - и снова забываешь написать заявление.'),
('О чём мечтаете?','О дальнем рейсе. Пока получается только до следующей цистерны.'),
('Как экипаж?','Один хочет к Юпитеру. Другой - в отпуск. Оба пока у мусорного контейнера.'),
('Есть опасные находки?','Всё, на чём написано «совершенно безопасно», проверяем дважды.'),
('Почему оставили этот предмет?','Мы убираем мусор, а не всё, что кто-то не успел подобрать.'),
('Артефакт не заберёте?','Нет. У нас и с обычным чайником отношения достаточно загадочные.'),
('Можно к вам в команду?','Умеешь ловить улетающий обед и не спорить с инструкцией? Уже неплохое начало.'),
('У вас тут романтика!','Есть немного. Обычно между техническим обслуживанием и выносом отходов.'),
('Почему стоите на краю системы?','Смену закончили. Ждём, пока вселенная снова намусорит.'),
('Куда дальше?','Туда, где после великих событий забыли вынести мусор.'),
('Передайте привет экипажу','Передам. Только тихо: у нас один наконец уснул.')]
extra=json.loads(Path(r'S:\SR\Debris\dialogue-additions.normalized.json').read_text(encoding='utf-8'))
topics=original+[(o['player'],o['npc']) for o in extra['ship_dialogue_additions']]
topics=[(q.replace('\u2014','-'),a.replace('\u2014','-').replace('У каждой станции своя музыка; у нашей','У каждого корабля своя музыка; у нашего')) for q,a in topics]
# Game fonts lack guillemets: straight quotes (previously applied afterwards by fix_quotes.py).
topics=[(q.replace('«','"').replace('»','"'),a.replace('«','"').replace('»','"')) for q,a in topics]
script='Mod_SmartLootServiceV2'
lang=parse_blockpar('Script ^{\n}\n')
def ct(key,text):
 key=str(len(lang.find_node('Script/'+script).parameters)) if any(c.name==script for c in lang.find_node('Script').children) else '0'
 lang.ensure_node('Script/'+script).set_parameter(key,text,create=True)
 return "CT('Script."+script+'.'+key+"')"
obj('Operations',Name='Global',**{'Code.Type':'Global','Code':['GRun();']})
for array_name in ['SL_ids','SL_dest','SL_qturn','SL_used','SL_system']:
 obj('Variables',Name=array_name,**{'Var.Type':'Array','Init':'0','Global':True})
for int_name in ['SL_started','SL_first','SL_busy','SL_busy_turn','SL_cursor','SL_planet_cursor','SL_scan_complete','SL_warned']:
 obj('Variables',Name=int_name,**{'Var.Type':'Int','Init':'0','Global':True})
obj('Operations',Name='FleetTurn',**{'Code.Type':'Turn','Code':(ROOT/'src/fleet.rscript').read_text(encoding='utf-8').splitlines()+['ServiceTurn();']})
state_id=obj('States',Name='ServiceIdle',Move=0,OnTalk='CrewTalk',OnActCode='[t_OnTakingDamage,t_OnTakingDamageMi,t_OnTakingDamageEn,t_OnTakingDamageSp|0]\r\nScriptItemActParam(0);')
star_id=obj('Stars',Name='ServiceStar')
player_start_id=obj('Ships',Name='PlayerStart',**{'+Star':'ServiceStar'})
planet_id=obj('Planets',Name='ServicePlanet',**{'+Star':'ServiceStar'})
fleet_group_id=obj('Groups',Name='ServiceGroup',AddPlayer=False,CntShipMin=0,CntShipMax=0,**{'+Planet':'ServicePlanet','+State':0})
obj('Dialogs',Name='CrewTalk')
obj('DialogMessages',Name='Menu',Msg="DText("+ct('Welcome','Служба уборки на связи. Как у тебя смена?')+");\n"+'\n'.join("DAdd('"+str(i)+"');" for i in range(len(topics)+1)),**{'DMsg.Num':'0'})
for i,(question,answer) in enumerate(topics):
 obj('DialogAnswers',Name='topic'+str(i),Msg='DAnswer('+ct('Q'+str(i),question)+');',**{'AMsg.Num':str(i)})
 obj('DialogMessages',Name='Reply'+str(i),Msg='DText('+ct('A'+str(i),answer)+");\nDAdd('"+str(len(topics)+1)+"');\nDAdd('"+str(len(topics))+"');",**{'DMsg.Num':str(i+1)})
obj('DialogAnswers',Name='Goodbye',Msg='DAnswer('+ct('Goodbye','Удачной смены')+');',**{'AMsg.Num':str(len(topics))})
obj('DialogAnswers',Name='Back',Msg='DAnswer('+ct('Back','Поговорим ещё')+');',**{'AMsg.Num':str(len(topics)+1)})
# Compiler derives graph flow from explicit links, keeping answer text separate.
links=[]
def link(a,b):links.append({'Type':'TGraphLink','Begin':a,'End':b,'Nom':0,'Arrow':True})
link(planet_id,star_id)
link(player_start_id,star_id)
link(fleet_group_id,planet_id)
link(fleet_group_id,state_id)
link(group['Dialogs'][0]['#'],messages[0]['#'])
for i in range(len(topics)):link(group['DialogAnswers'][i]['#'],messages[i+1]['#'])
link(group['DialogAnswers'][-1]['#'],messages[0]['#'])
obj('DialogMessages',Name='Bye',Msg='DText('+ct('Bye','И тебе чистой орбиты.')+");\nDAnswer('exit');",**{'DMsg.Num':str(len(topics)+1)})
link(group['DialogAnswers'][-2]['#'],messages[-1]['#'])
p={k:copy.deepcopy(v) for k,v in base.items() if k not in ('Visual.Objects','Visual.Links','Rect.Text')}
p.update(ScriptName=script,ScriptFileOut='',ScriptTextOut='',**{'Visual.Objects':[group],'Visual.Links':links,'Rect.Text':[]})
for key,text in [('SCV01','SCV-01 Работяга'),('SCV02','SCV-02 Скат'),('SCV03','SCV-03 Косточка'),
                 ('NoLoader','SmartLootAI: служба уборки не запущена.<br><br>Нативная часть мода не загрузилась. Обычно это значит, что не установлен XenoNativeLoader: рядом с Rangers.exe должны лежать dsound.dll и XenoCore.dll из архива мода. Реже - версия игры не поддерживается или другой нативный мод уже изменил те же места игры.<br><br>Мусорщики и умная уборка отключены, сохранение не пострадает. Распакуйте архив мода в папку игры целиком и перезапустите игру. Подробности - в файле XenoNative.log в папке игры.')]:
 lang.ensure_node('Script/'+script).set_parameter(key,text,create=True)
(ROOT/'src'/f'{script}.rson').write_text(json.dumps(p,ensure_ascii=False,indent=2),encoding='utf-8')
(ROOT/'RuntimeMod/CFG/Rus').mkdir(parents=True,exist_ok=True)
lang.save(ROOT/'src/CrewLang.txt',encoding='cp1251')
# English: same keys, texts from src/crew_eng.json (SR2-style localized names).
ru_node=lang.find_node('Script/'+script)
ru_keys=[p.key for p in ru_node.parameters]
eng=json.loads((ROOT/'src/crew_eng.json').read_text(encoding='utf-8'))
assert set(eng)==set(ru_keys),('eng keys mismatch',sorted(set(ru_keys)^set(eng)))
bad=[k for k,v in eng.items() if any(ord(c)>127 for c in v)]
assert not bad,('non-ASCII in English text',bad)
lang_en=parse_blockpar('Script ^{\n}\n')
for k in ru_keys:lang_en.ensure_node('Script/'+script).set_parameter(k,eng[k],create=True)
lang_en.save(ROOT/'src/CrewLang_Eng.txt',encoding='cp1251')
print(f'Generated {len(topics)} topics plus goodbye, 36 answers, localized RSON; station-only lines excluded.')
