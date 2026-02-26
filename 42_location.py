import os
import sys
import datetime as dt
from collections import defaultdict
import requests
from dotenv import load_dotenv

load_dotenv()

FT_UID = os.getenv("FT_UID")
FT_SECRET = os.getenv("FT_SECRET")
FT_API_BASE = os.getenv("FT_API_BASE", "https://api.intra.42.fr")
FT_LOGIN = os.getenv("FT_LOGIN")

if not FT_UID or not FT_SECRET:
    print("Erreur: FT_UID ou FT_SECRET manquants dans le fichier .env")
    sys.exit(1)

def get_token():
    url = f"{FT_API_BASE}/oauth/token"
    data = {
        "grant_type": "client_credentials",
        "client_id": FT_UID,
        "client_secret": FT_SECRET
    }
    resp = requests.post(url, data=data, timeout=10)
    if resp.status_code != 200:
        print("Erreur token:", resp.status_code, resp.text)
        sys.exit(1)
    return resp.json()["access_token"]

def get_all_user_locations(login, token):
    """Récupère TOUTES les locations d'un utilisateur depuis le début"""
    url = f"{FT_API_BASE}/v2/users/{login}/locations"
    headers = {"Authorization": f"Bearer {token}"}
    
    all_locations = []
    page = 1
    
    print(f"📡 Récupération de TOUTES les locations pour {login}...")
    print(f"⏳ Cela peut prendre du temps...\n")
    
    while True:
        params = {
            "page[size]": 100,
            "page[number]": page,
            "sort": "-begin_at"
        }
        
        try:
            resp = requests.get(url, headers=headers, params=params, timeout=30)
            
            if resp.status_code != 200:
                print(f"❌ Erreur page {page}: {resp.status_code}")
                if resp.status_code == 404:
                    print(f"   Login '{login}' introuvable")
                elif resp.status_code == 429:
                    print(f"   Rate limit atteint, attendez un peu...")
                break
            
            data = resp.json()
            
            if not data or len(data) == 0:
                break
                
            page_locations = data if isinstance(data, list) else []
            all_locations.extend(page_locations)
            
            print(f"  📄 Page {page}: {len(page_locations)} locations | Total: {len(all_locations)}")
            
            if len(page_locations) < 100:
                print(f"\n✅ Toutes les pages récupérées!")
                break
                
            page += 1
            
            if page > 1000:
                print("⚠️  Limite de 1000 pages atteinte")
                break
                
        except requests.exceptions.Timeout:
            print(f"⏱️  Timeout à la page {page}, nouvel essai...")
            continue
        except Exception as e:
            print(f"❌ Erreur inattendue: {e}")
            break
    
    return all_locations

def parse_host(host):
    """Extrait la zone et le numéro de rangée d'un nom de poste"""
    # Format attendu: z1r1p1, z2r3p15, etc.
    if not host or host == "unknown":
        return None, None, None
    
    try:
        # Extraction: z[zone]r[row]p[poste]
        if 'z' in host and 'r' in host and 'p' in host:
            parts = host.lower().replace('z', '').replace('r', ' ').replace('p', ' ').split()
            if len(parts) >= 3:
                zone = int(parts[0])
                row = int(parts[1])
                poste = int(parts[2])
                return zone, row, poste
    except:
        pass
    
    return None, None, None

def analyze_locations(locations):
    """Analyse complète des locations"""
    stats_par_poste = defaultdict(float)
    total_heures = 0
    
    premiere_session = None
    derniere_session = None
    
    for loc in locations:
        try:
            begin = dt.datetime.fromisoformat(loc["begin_at"].replace("Z", "+00:00"))
            end_str = loc.get("end_at")
            
            if end_str is None:
                end = dt.datetime.now(dt.timezone.utc)
            else:
                end = dt.datetime.fromisoformat(end_str.replace("Z", "+00:00"))
            
            duration = (end - begin).total_seconds() / 3600
            
            host = loc.get("host", "unknown") or "unknown"
            stats_par_poste[host] += duration
            total_heures += duration
            
            if premiere_session is None or begin < premiere_session:
                premiere_session = begin
            if derniere_session is None or begin > derniere_session:
                derniere_session = begin
            
        except (KeyError, ValueError, TypeError):
            continue
    
    return {
        'par_poste': dict(stats_par_poste),
        'total_heures': total_heures,
        'premiere_session': premiere_session,
        'derniere_session': derniere_session,
        'nb_sessions': len(locations)
    }

def print_stats_by_row(login, stats):
    """Affichage organisé par rangée (row)"""
    print(f"\n{'='*80}")
    print(f"🎯 STATISTIQUES COMPLÈTES - {login.upper()}")
    print(f"{'='*80}")
    
    if stats['premiere_session']:
        duree_totale = (dt.datetime.now(dt.timezone.utc) - stats['premiere_session']).days
        print(f"📅 Période: {stats['premiere_session'].strftime('%Y-%m-%d')} → {stats['derniere_session'].strftime('%Y-%m-%d')}")
        print(f"📊 Durée: {duree_totale} jours ({duree_totale/365:.1f} ans)")
    
    print(f"🔢 Sessions: {stats['nb_sessions']}")
    print(f"⏱️  Total: {stats['total_heures']:.1f}h ({stats['total_heures']/24:.1f} jours)")
    
    if not stats['par_poste']:
        print("\n❌ Aucune session trouvée")
        return
    
    # Organisation par zone et rangée
    postes_par_zone_row = defaultdict(lambda: defaultdict(list))
    postes_inconnus = []
    
    for host, heures in stats['par_poste'].items():
        zone, row, poste = parse_host(host)
        
        if zone is not None and row is not None:
            postes_par_zone_row[zone][row].append((host, heures))
        else:
            postes_inconnus.append((host, heures))
    
    # Affichage par zone et rangée
    print(f"\n{'='*80}")
    print(f"🗺️  TEMPS PAR POSTE - ORGANISÉ PAR RANGÉE")
    print(f"{'='*80}")
    
    for zone in sorted(postes_par_zone_row.keys()):
        print(f"\n{'─'*80}")
        print(f"📍 ZONE {zone}")
        print(f"{'─'*80}")
        
        for row in sorted(postes_par_zone_row[zone].keys()):
            postes = sorted(postes_par_zone_row[zone][row], key=lambda x: x[0])
            total_row = sum(h for _, h in postes)
            
            print(f"\n  ┌─ Row {row} (Total: {total_row:.1f}h)")
            print(f"  │")
            
            for host, heures in postes:
                jours = heures / 24
                pct = (heures / stats['total_heures'] * 100) if stats['total_heures'] > 0 else 0
                print(f"  │  {host:<15} │ {heures:>8.1f}h │ {jours:>7.1f}j │ {pct:>5.1f}%")
            
            print(f"  └─")
    
    # Postes non identifiés
    if postes_inconnus:
        print(f"\n{'─'*80}")
        print(f"❓ POSTES NON IDENTIFIÉS / AUTRES")
        print(f"{'─'*80}")
        
        for host, heures in sorted(postes_inconnus, key=lambda x: x[1], reverse=True):
            jours = heures / 24
            pct = (heures / stats['total_heures'] * 100) if stats['total_heures'] > 0 else 0
            print(f"  {host:<15} │ {heures:>8.1f}h │ {jours:>7.1f}j │ {pct:>5.1f}%")
    
    # TOP 20 général
    print(f"\n{'='*80}")
    print(f"🏆 TOP 20 POSTES LES PLUS UTILISÉS")
    print(f"{'='*80}")
    print(f"{'#':<4} {'Poste':<15} │ {'Heures':>10} │ {'Jours':>8} │ {'%':>6}")
    print("─" * 80)
    
    for idx, (host, heures) in enumerate(sorted(stats['par_poste'].items(), key=lambda x: x[1], reverse=True)[:20], 1):
        jours = heures / 24
        pct = (heures / stats['total_heures'] * 100) if stats['total_heures'] > 0 else 0
        print(f"{idx:<4} {host:<15} │ {heures:>10.1f}h │ {jours:>7.1f}j │ {pct:>5.1f}%")

def main(): 

    	

    query_string = os.environ.get("QUERY_STRING", "Inconnu")


    print("Content-Type: text/html; charset=utf-8")
    print("")

    html_content = f"""<html>
        <head>
            <style>
                body {{
                    background: #1a1a1a;
                    color: white;
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    margin: 0;
                    font-family: sans-serif;
                    flex-direction: column;
                    }}
                    h1 {{ color: #16d705; }}
                    p {{ color: #ffffff; }}
            </style>
        </head>
        <body>
        <center>
            <h1>Logtime Location of {query_string}</h1>
        </center>
        <pre>Données traitées avec succès !</pre>
        <pre>
    """

    print(html_content)

    login = FT_LOGIN
            
    if not login:
        if len(sys.argv) >= 2:
            login = sys.argv[1]
        else:
            print("Erreur: Login manquant")
            sys.exit(1)
    
    print(f"🚀 Démarrage de l'analyse pour: {login}\n")
    
    token = get_token()
    locations = get_all_user_locations(login, token)
    
    if not locations:
        print(f"\n❌ Aucune location trouvée pour {login}")
        return
    
    print(f"\n📊 Analyse de {len(locations)} sessions...")
    stats = analyze_locations(locations)
    print_stats_by_row(login, stats)
    
    print(f"\n{'='*80}")
    print("✅ Analyse terminée!")
    print("</pre></body></html>")

if __name__ == "__main__":
    main()
