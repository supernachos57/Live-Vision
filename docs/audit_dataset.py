import csv, hashlib, json, itertools
from pathlib import Path
import numpy as np
from PIL import Image, ImageOps, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT/'data/GroceryStoreDataset/dataset'
import argparse
parser = argparse.ArgumentParser(description='Regenerate the bounded dataset inventory and visual candidates; does not change split membership.')
parser.add_argument('--output', type=Path, required=True, help='Directory for generated manifests and review images')
OUT = parser.parse_args().output
OUT.mkdir(parents=True, exist_ok=True)
records=[]
for split in ('train','val','test'):
    selected=[]
    with (DATA/f'{split}.txt').open(newline='') as f:
        for path,fine,coarse in csv.reader(f):
            coarse=int(coarse)
            if coarse not in (1,2,4): continue
            label={1:0,2:1,4:2}[coarse]
            assert int(fine)==[5,6,8][label]
            p=DATA/path
            with Image.open(p) as im:
                rgb=im.convert('RGB')
                small=np.asarray(rgb.resize((32,32)), dtype=np.float32)/255
                gray=np.asarray(rgb.convert('L').resize((9,8)),dtype=np.int16)
                dh=(gray[:,1:]>gray[:,:-1]).reshape(-1)
                pixelhash=hashlib.sha256(str(rgb.size).encode()+rgb.tobytes()).hexdigest()
                size=list(rgb.size)
            rec=dict(split=split,path=path,fine_id=int(fine),coarse_id=coarse,label=label,
                     width=size[0],height=size[1],file_sha256=hashlib.sha256(p.read_bytes()).hexdigest(),
                     rgb_sha256=pixelhash)
            selected.append(rec)
            records.append((rec,small,dh))
    (OUT/f'{split}.txt').write_text(''.join(f"{r['path']}, {r['fine_id']}, {r['coarse_id']}\n" for r in selected),encoding='utf-8')
inventory=[r[0] for r in records]
groups={}
for field in ('file_sha256','rgb_sha256','path'):
    grouped={}
    for r in inventory: grouped.setdefault(r[field],[]).append(r['path'])
    groups[field]=[v for v in grouped.values() if len(v)>1]
summary=dict(selected_counts={s:sum(r['split']==s for r in inventory) for s in ('train','val','test')},
             duplicate_groups=groups,upstream_manifests={s:hashlib.sha256((DATA/f'{s}.txt').read_bytes()).hexdigest() for s in ('train','val','test')})
(OUT/'inventory.json').write_text(json.dumps(inventory,indent=2)+'\n')
(OUT/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')

def thumb(draw,canvas,path,x,y,w=170,h=140):
    with Image.open(DATA/path) as im:
        im=ImageOps.contain(im.convert('RGB'),(w,h))
        canvas.paste(im,(x,y))
    parts=Path(path).parts
    draw.text((x,y+h+2),parts[0]+' '+Path(path).stem,fill='black')

candidate_records=[]
for label,name in enumerate(('avocado','banana','lemon')):
    subset=[r for r in records if r[0]['label']==label]
    # Review the top two pairs by each heuristic for each split pair.
    chosen={}
    for sa,sb in (('train','val'),('train','test'),('val','test')):
        pairs=[]
        for a,b in itertools.combinations(subset,2):
            if {a[0]['split'],b[0]['split']}!={sa,sb}: continue
            pairs.append((float(np.mean((a[1]-b[1])**2)),int(np.count_nonzero(a[2]!=b[2])),a[0],b[0]))
        for key in (0,1):
            for pair in sorted(pairs,key=lambda p:p[key])[:2]: chosen[(pair[2]['path'],pair[3]['path'])]=pair
    pairs=list(chosen.values())
    canvas=Image.new('RGB',(750,180*len(pairs)+30),'white'); draw=ImageDraw.Draw(canvas)
    draw.text((10,8),name+' cross-split candidates; similarity is not proof of duplication',fill='black')
    for i,(mse,dist,a,b) in enumerate(pairs):
        y=30+i*180
        thumb(draw,canvas,a['path'],8,y);thumb(draw,canvas,b['path'],195,y)
        draw.text((380,y+30),f'RGB MSE={mse:.5f}\ndHash distance={dist}',fill='black')
        candidate_records.append(dict(class_name=name,left=a['path'],right=b['path'],mse=mse,dhash_distance=dist))
    canvas.save(OUT/f'{name}-candidates.png')
    tests=[r[0] for r in subset if r[0]['split']=='test']
    canvas=Image.new('RGB',(1080,170*((len(tests)+5)//6)+30),'white');draw=ImageDraw.Draw(canvas)
    draw.text((10,8),name+' test images: data-integrity/grouping audit only; no model predictions',fill='black')
    for i,r in enumerate(tests): thumb(draw,canvas,r['path'],(i%6)*180,30+(i//6)*170)
    canvas.save(OUT/f'{name}-test.png')
(OUT/'similarity-candidates.json').write_text(json.dumps(candidate_records,indent=2)+'\n')
print(json.dumps(summary,indent=2))
print('Candidate pairs:',len(candidate_records))
